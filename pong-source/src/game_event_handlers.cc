#include "game_event_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "common_handlers.h"
#include "leaderboard.h"
#include "matchmaking.h"
#include "pong_loggers.h"
#include "pong_types.h"


// USE_JSON 은 CMakeLists.txt 에서 세팅됩니다.
#ifndef USE_JSON
#include "pong_messages.pb.h"
#endif


DECLARE_string(app_flavor);


namespace pong {

// Player 들의 승/패를 1 증가 시킵니다.
void FetchAndUpdateMatchRecord(const string& winner_id,
                               const string& loser_id) {
  Ptr<User> winner = User::FetchById(winner_id);
  Ptr<User> loser = User::FetchById(loser_id);

  if(not winner) {
    LOG(ERROR) << "Cannot find winner's id in db: id=" << winner_id;
    return;
  }

  if(not loser) {
    LOG(ERROR) << "Cannot find loser's id in db: id=" << loser_id;
    return;
  }

  winner->SetWinCount(winner->GetWinCount() + 1);
  loser->SetLoseCount(loser->GetLoseCount() + 1);
}


// game/lobby_handlers.cc 에 서로 동일한 함수명을 갖는 것들은
// unnamed namespace 로 감쌉니다.
namespace {

void FreeUser(const Ptr<Session> &session);

// 새 클라이언트가 접속하여 세션이 열릴 때 불리는 함수
void OnSessionOpened(const Ptr<Session> &session) {
  // 세션 접속  Activity Log 를 남깁니다.
  logger::SessionOpened(to_string(session->id()), WallClock::Now());
}


// 세션이 닫혔을 때 불리는 함수
void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason) {
  // 세션 닫힘 Activity Log 를 남깁니다.
  logger::SessionClosed(to_string(session->id()), WallClock::Now());
  // 세션을 초기과 합니다.
  FreeUser(session);
}


// TCP 연결이 끊기면 불립니다.
void OnTransportTcpDetached(const Ptr<Session> &session) {
  string id;
  session->GetFromContext("id", &id);
  LOG_IF(INFO, not id.empty()) << "TCP disconnected: id=" << id;
  // 세션을 초기과 합니다.
  FreeUser(session);
}


// 세션을 정리합니다.
void FreeUser(const Ptr<Session> &session) {
  // 유저를 정리하기 위한 Context 를 읽어옵니다.
  string opponent_id;
  string my_id;
  session->GetFromContext("opponent", &opponent_id);
  session->GetFromContext("id", &my_id);

  // Session Context 를 초기화 합니다.
  session->SetContext(Json());

  // 로그아웃하고 세션을 종료합니다.
  if (not my_id.empty()) {
    auto logout_cb = [](const string &id, const Ptr<Session> &session,
                        bool success) {
      if (success) {
        LOG(INFO) << "Logged out(local) by session close: id=" << id;
      }
    };
    AccountManager::SetLoggedOutAsync(my_id, logout_cb);
  }

  // 대전 상대가 있는 경우, 상대가 승리한 것으로 처리하고 로비서버로 보냅니다.
  if (opponent_id.empty()) {
    return;
  }
  Ptr<Session> opponent_session = AccountManager::FindLocalSession(opponent_id);
  if (not opponent_session || not opponent_session->IsTransportAttached()) {
    return;
  }

  Event::Invoke(bind(&FetchAndUpdateMatchRecord, opponent_id, my_id));

  IncreaseCurWinCount(opponent_id);
  ResetCurWinCount(my_id);

#ifdef USE_JSON
  opponent_session->SendMessage("result", MakeResponse("win"),
                                kDefaultEncryption);
#else
  Ptr<FunMessage> msg(new FunMessage);
  GameResultMessage *result_msg = msg->MutableExtension(game_result);
  result_msg->set_result("win");
  opponent_session->SendMessage("result", msg);
#endif

  MoveServerByTag(opponent_session, "lobby");

  opponent_session->DeleteFromContext("opponent");
  FreeUser(opponent_session);
}

}  // unnamed namesapce


void HandleReadySignal(const Ptr<Session> &session) {
  session->AddToContext("ready", 1);
  string opponent_id;
  session->GetFromContext("opponent", &opponent_id);
  Ptr<Session> opponent_session = AccountManager::FindLocalSession(opponent_id);
  // 상대의 상태를 확인합니다.
  if (opponent_session && opponent_session->IsTransportAttached()) {
    int64_t is_opponent_ready = 0;
    opponent_session->GetFromContext("ready", &is_opponent_ready);
    if (is_opponent_ready == 1) {
      // 둘 다 준비가 되었습니다. 시작 신호를 보냅니다.
#ifdef USE_JSON
      Json response = MakeResponse("ok");
      session->SendMessage("start", response);
      opponent_session->SendMessage("start", response);
#else
      Ptr<FunMessage> msg(new FunMessage);
      GameStartMessage *start_msg = msg->MutableExtension(game_start);
      start_msg->set_result("ok");
      opponent_session->SendMessage("start", msg);
#endif
    }
  } else {
    // 상대가 접속을 종료했습니다.
#ifdef USE_JSON
    session->SendMessage("match", MakeResponse("opponent disconnected"),
                         kDefaultEncryption);
#else
    Ptr<FunMessage> msg(new FunMessage);
    LobbyMatchReply *match_msg = msg->MutableExtension(lobby_match_repl);
    match_msg->set_result("opponent disconnected");
    session->SendMessage("match", msg);
#endif
    return;
  }
}


void HandleResultRequest(const Ptr<Session> &session) {
  // 패배한 쪽만 result를 보내도록 되어있습니다.

  // 내 아이디를 가져옵니다.
  string my_id;
  session->GetFromContext("id", &my_id);

  // 상대방의 아이디와 세션을 가져옵니다.
  string opponent_id;
  session->GetFromContext("opponent", &opponent_id);
  Ptr<Session> opponent_session = AccountManager::FindLocalSession(opponent_id);

  FetchAndUpdateMatchRecord(opponent_id, my_id);

  if (opponent_session && opponent_session->IsTransportAttached()) {
    // 상대에게 승리했음을 알립니다.
#ifdef USE_JSON
    opponent_session->SendMessage("result", MakeResponse("win"));
#else
    Ptr<FunMessage> msg(new FunMessage);
    GameResultMessage *result_msg = msg->MutableExtension(game_result);
    result_msg->set_result("win");
    opponent_session->SendMessage("result", msg);
#endif
    IncreaseCurWinCount(opponent_id);
  }

  // 패배 확인 메세지를 보냅니다.
#ifdef USE_JSON
  session->SendMessage("result", MakeResponse("lose"));
#else
  Ptr<FunMessage> msg(new FunMessage);
  GameResultMessage *result_msg = msg->MutableExtension(game_result);
  result_msg->set_result("lose");
  opponent_session->SendMessage("result", msg);
#endif
  ResetCurWinCount(my_id);

  // 각각 상대방에 대한 정보를 삭제합니다.
  opponent_session->DeleteFromContext("opponent");
  session->DeleteFromContext("opponent");

  // 두 플레이어를 lobby서버로 이동시킵니다.
  MoveServerByTag(opponent_session, "lobby");
  MoveServerByTag(session, "lobby");

  FreeUser(opponent_session);
  FreeUser(session);
}


#ifdef USE_JSON

// 게임 플레이 준비 메시지를 받으면 불립니다.
void OnReadySignal(const Ptr<Session> &session, const Json &/*message*/) {
  HandleReadySignal(session);
}


// 릴레이 메시지를 받으면 불립니다. TCP, UDP 둘 다 이 함수로 처리합니다.
void OnRelayRequested(const Ptr<Session> &session, const Json &message) {
  string opponent_id;
  session->GetFromContext("opponent", &opponent_id);
  Ptr<Session> opponent_session = AccountManager::FindLocalSession(opponent_id);
  if (opponent_session && opponent_session->IsTransportAttached()) {
    LOG(INFO) << "message relay: session_id=" << session->id();
    opponent_session->SendMessage("relay", message);
  }
}


// 결과 요청 메시지를 받으면 불립니다.
void OnResultRequested(const Ptr<Session> &session, const Json &/*message*/) {
  HandleResultRequest(session);
}

#else

// 게임 플레이 준비 메시지를 받으면 불립니다.
void OnReadySignal(
    const Ptr<Session> &session, const Ptr<FunMessage> &/*message*/) {
  HandleReadySignal(session);
}


// 릴레이 메시지를 받으면 불립니다. TCP, UDP 둘 다 이 함수로 처리합니다.
void OnRelayRequested(
    const Ptr<Session> &session, const Ptr<FunMessage> &message) {
  string opponent_id;
  session->GetFromContext("opponent", &opponent_id);
  Ptr<Session> opponent_session = AccountManager::FindLocalSession(opponent_id);
  if (opponent_session && opponent_session->IsTransportAttached()) {
    LOG(INFO) << "message relay: session_id=" << session->id();
    opponent_session->SendMessage("relay", message);
  }

}


// 결과 요청 메시지를 받으면 불립니다.
void OnResultRequested(
    const Ptr<Session> &session, const Ptr<FunMessage> &/*message*/) {
  HandleResultRequest(session);
}

#endif


// 게임 서버 핸들러들을 등록합니다.
void RegisterGameEventHandlers() {
  HandlerRegistry::Install2(OnSessionOpened, OnSessionClosed);
  HandlerRegistry::RegisterTcpTransportDetachedHandler(OnTransportTcpDetached);

#ifdef USE_JSON
  HandlerRegistry::Register("ready", OnReadySignal);
  HandlerRegistry::Register("relay", OnRelayRequested);
  HandlerRegistry::Register("result", OnResultRequested);
#else
  HandlerRegistry::Register2("ready", OnReadySignal);
  HandlerRegistry::Register2("relay", OnRelayRequested);
  HandlerRegistry::Register2("result", OnResultRequested);
#endif
}

}  // namespace pong
