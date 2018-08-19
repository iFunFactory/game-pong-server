#include "game_event_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "common_handlers.h"
#include "leaderboard.h"
#include "matchmaking.h"
#include "pong_loggers.h"
#include "pong_types.h"

#include "pong_messages.pb.h"

#include "game_session.h"


DECLARE_string(app_flavor);

DECLARE_uint64(tcp_json_port);
DECLARE_uint64(udp_json_port);
DECLARE_uint64(http_json_port);
DECLARE_uint64(tcp_protobuf_port);
DECLARE_uint64(udp_protobuf_port);
DECLARE_uint64(http_protobuf_port);


namespace pong {

// game/lobby_handlers.cc 에 서로 동일한 함수명을 갖는 것들은
// unnamed namespace 로 감쌉니다.
namespace {

EncodingScheme the_scheme;

// 게임 세션 (=게임이 진행되는 방) 을 관리한다.
// Session -> GameSession 을 바로 얻을 수 있게 cache를 쓴다.
std::deque<int32_t> the_empty_game_cache_slots;
std::vector<boost::weak_ptr<GameSession>> the_game_cache{4096};

using UuidToGameSession = boost::unordered_map<
    Uuid, Ptr<GameSession>, boost::hash<Uuid>>;

boost::shared_mutex the_game_lock;
UuidToGameSession the_games;
UuidToGameSession the_session_id_to_game;


void _RegisterGame(const Ptr<GameSession> &game) {
  // game_lock 필요
  the_games.insert(std::make_pair(game->id_, game));
  the_game_cache[game->game_index_] = game;
}


void _UnregisterGame(const Ptr<GameSession> &game) {
  // game_lock 필요
  the_empty_game_cache_slots.push_back(game->game_index_);
  the_game_cache[game->game_index_].reset();
  the_games.erase(game->id_);
}


int32_t _GetEmptyGameCacheSlotIndex() {
  // game_lock 필요
  if (the_empty_game_cache_slots.empty()) {
    auto last_size = the_game_cache.size();
    auto new_size = last_size * 17 / 10;  // x1.7

    for (auto i = last_size; i < new_size; ++i) {
      the_empty_game_cache_slots.push_back(i);
    }

    the_game_cache.resize(new_size);
  }

  int32_t cache_index = the_empty_game_cache_slots.front();
  the_empty_game_cache_slots.pop_front();

  return cache_index;
}


inline void _SetSessionToGame(
    const Ptr<Session> &session, const Ptr<GameSession> &game) {
  if (the_session_id_to_game.find(session->id())
        != the_session_id_to_game.end()) {
      LOG(ERROR) << "Session(" << session->id() << ") already in map";
      return;
  }

  the_session_id_to_game.insert(std::make_pair(session->id(), game));
}



inline Ptr<GameSession> _GetGame(const Ptr<Session> &session) {
  auto found = the_session_id_to_game.find(session->id());
  if (found != the_session_id_to_game.end()) {
    return found->second;
  }
  return nullptr;
}


Ptr<GameSession> GetGame(const Ptr<Session> &session) {
  // NOTE: session 문맥에서 실행해야 한다.
  // the_game_lock을 가지고 들어오면 데드락에 걸릴 수 있다.
  Ptr<GameSession> game;

  for (;;) {
    int64_t index = -1;

    if (not session->GetFromContext("game_index", &index)) {
      boost::unique_lock<boost::shared_mutex> lock(the_game_lock);
      game = _GetGame(session); // TODO(implement)
      lock.unlock();

      if (game) {
        session->AddToContext("game_index", game->game_index_);
        session->AddToContext("game_id", to_string(game->id_));
      }

      break;
    }

    std::string game_id;

    if (not session->GetFromContext("game_id", &game_id)) {
      session->GetContext().RemoveAttribute("game_index");
      game = GetGame(session);
      break;
    }

    game = the_game_cache[index].lock();

    if (game && to_string(game->id_) == game_id) {
      break;
    }

    session->GetContext().RemoveAttribute("game_index");
    session->GetContext().RemoveAttribute("game_id");
    game = GetGame(session);
    break;
  }

  return game;
}


// 클라이언트가 다른 서버에서 이동해 왔을 때 불립니다.
void OnClientRedirected(
    const std::string &account_id, const Ptr<Session> &session, bool success,
    const std::string &extra_data) {
  if (not success) {
    LOG(WARNING) << "Client redirection failed. Blocked by the engine: id="
                 << account_id;
    session->Close();
    return;
  }

  // 이전 서버의 Session Context 를 적용합니다.
  Json context;
  context.FromString(extra_data);
  if (context.HasAttribute("context", fun::Json::kObject)) {
    boost::mutex::scoped_lock lock(*session);
    session->SetContext(context["context"]);
  }

  Json game_context;
  if (context.HasAttribute("game_context", fun::Json::kObject)) {
    game_context = context["game_context"];
  } else {
    LOG(ERROR) << "Client(id=" << account_id << ") does not have match_data";
    session->Close();
    return;
  }

  if (not game_context.HasAttribute("game_id", fun::Json::kString) ||
      not game_context.HasAttribute("users", fun::Json::kArray)) {
    LOG(ERROR) << "game_context does not have required field(s).";
    session->Close();
    return;
  }

  fun::Uuid game_id;
  try {
    game_id = boost::lexical_cast<Uuid>(game_context["game_id"].GetString());
  } catch (const boost::bad_lexical_cast &) {
    LOG(ERROR) << "game_context has invalid game_id: "
               << game_context["game_id"].GetString();
    session->Close();
    return;
  }

  const size_t user_len = game_context["users"].Size();
  std::vector<std::string> users;
  users.reserve(user_len);
  for (size_t i = 0; i < user_len; ++i) {
    if (game_context["users"][i].IsString()) {
      users.emplace_back(game_context["users"][i].GetString());
    } else {
      LOG(ERROR) << "game_context has non-string user";
      session->Close();
      return;
    }
  }

  LOG(INFO) << "Client redirected: id=" << account_id << ", game=" << game_id;

  // TODO(jinuk): match_data 를 토대로 매치 생성
  fun::Event::Invoke([session, game_id, users]() {
      // game 에 해당하는 EventTag 로 실행한다.
      boost::unique_lock<boost::shared_mutex> lock(the_game_lock);
      // 유저 추가하고 리턴
      auto found = the_games.find(game_id);
      Ptr<GameSession> game;
      if (found != the_games.end()) {
        game = found->second;
        if (game->Join(session)) {
          _SetSessionToGame(session, game);
        }
      } else {
        const int32_t cache_index = _GetEmptyGameCacheSlotIndex();
        game = GameSession::Create(the_scheme, cache_index, game_id, users);

        // 신규 생성한 방이라서 [정원초과, 이미 시작함] 에러가 생기지는 않음.
        if (game->Join(session)) {
          _RegisterGame(game);
          _SetSessionToGame(session, game);
        }
      }
    }, game_id);
}


// 새 클라이언트가 접속하여 세션이 열릴 때 불리는 함수
void OnSessionOpened(const Ptr<Session> &session) {
  // 세션 접속  Activity Log 를 남깁니다.
  logger::SessionOpened(to_string(session->id()), WallClock::Now());
}


// 세션이 닫혔을 때 불리는 함수
void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason,
                     EncodingScheme encoding) {
  // 세션 닫힘 Activity Log 를 남깁니다.
  logger::SessionClosed(to_string(session->id()), WallClock::Now());

  std::string uid = AccountManager::FindLocalAccount(session);
  if (uid.empty()) {
    return;
  }

  // 세션에 해당하는 게임이 있는 경우 종료 처리
  Ptr<GameSession> game = GetGame(session);
  if (game) {
    fun::Event::Invoke([game, uid]() {
      if (game->Leave(uid)) {
        TerminateGame(game->id_);
      }
    }, game->id_);
  }

  // 로그아웃 시킨다.
  AccountManager::SetLoggedOutAsync(uid,
    [](const string &uid, const Ptr<Session>&, bool success){
      if (not success) {
        LOG(ERROR) << "Failed to logout User(" << uid << ")";
      }
    });
}


// TCP 연결이 끊기면 불립니다.
void OnTransportTcpDetached(const Ptr<Session> &session,
                            EncodingScheme encoding) {
  Ptr<GameSession> game = GetGame(session);
  if (not game) {
    LOG(ERROR) << "Session(" << session->id() << "; "
               << AccountManager::FindLocalAccount(session)
               << "): TCP detached -  game not found";
    session->Close();
    return;
  }

  fun::Event::Invoke([game, session]() {
      game->SetDetached(session);
    }, game->id_);
}


void HandleReadySignal(const Ptr<Session> &session, EncodingScheme encoding) {
  Ptr<GameSession> game = GetGame(session);
  if (not game) {
    LOG(ERROR) << "Session(" << session->id() << "; "
               << AccountManager::FindLocalAccount(session)
               << "): ready - game not found";
    session->Close();
    return;
  }

  fun::Event::Invoke([game, session]() {
      game->SetReady(session);
    }, game->id_);
}


void HandleResultRequest(const Ptr<Session> &session, EncodingScheme encoding) {
  // 패배한 쪽만 result를 보내도록 되어있습니다.
  Ptr<GameSession> game = GetGame(session);
  if (not game) {
    LOG(ERROR) << "Session(" << session->id() << "; "
               << AccountManager::FindLocalAccount(session)
               << "): result - game not found";
    session->Close();
    return;
  }

  fun::Event::Invoke([game, session]() {
      game->SetResult(session);
    }, game->id_);
}


////////////////////////////////////////////////////////////////////////////////
//
// JSON 메시지 핸들러들
//
////////////////////////////////////////////////////////////////////////////////

// 게임 플레이 준비 메시지를 받으면 불립니다.
void OnReadySignal(const Ptr<Session> &session, const Json &/*message*/) {
  HandleReadySignal(session, kJsonEncoding);
}


// 릴레이 메시지를 받으면 불립니다. TCP, UDP 둘 다 이 함수로 처리합니다.
void OnRelayRequested(const Ptr<Session> &session, const Json &message) {
  Ptr<GameSession> game = GetGame(session);
  if (not game) {
    LOG(ERROR) << "Session(" << session->id() << "; "
               << AccountManager::FindLocalAccount(session)
               << "): relay -  game not found";
    session->Close();
    return;
  }

  fun::Event::Invoke([session, message, game]() {
      game->RelayMessage(session, message);
    }, game->id_);
}


// 결과 요청 메시지를 받으면 불립니다.
void OnResultRequested(const Ptr<Session> &session, const Json &/*message*/) {
  HandleResultRequest(session, kJsonEncoding);
}


////////////////////////////////////////////////////////////////////////////////
//
// Protobuf 메시지 핸들러들
//
////////////////////////////////////////////////////////////////////////////////

// 게임 플레이 준비 메시지를 받으면 불립니다.
void OnReadySignal2(
    const Ptr<Session> &session, const Ptr<FunMessage> &/*message*/) {
  HandleReadySignal(session, kProtobufEncoding);
}


// 릴레이 메시지를 받으면 불립니다. TCP, UDP 둘 다 이 함수로 처리합니다.
void OnRelayRequested2(
    const Ptr<Session> &session, const Ptr<FunMessage> &message) {
  Ptr<GameSession> game = GetGame(session);
  if (not game) {
    LOG(ERROR) << "Session(" << session->id() << "; "
               << AccountManager::FindLocalAccount(session)
               << "): relay -  game not found";
    session->Close();
    return;
  }

  fun::Event::Invoke([session, message, game]() {
      game->RelayMessage(session, message);
    }, game->id_);
}


// 결과 요청 메시지를 받으면 불립니다.
void OnResultRequested2(
    const Ptr<Session> &session, const Ptr<FunMessage> &/*message*/) {
  HandleResultRequest(session, kProtobufEncoding);
}

}  // unnamed namesapce


void TerminateGame(const fun::Uuid &game_id) {
  boost::unique_lock<boost::shared_mutex> lock(the_game_lock);
  auto it = the_games.find(game_id);
  if (it == the_games.end()) {
    return;
  }
  _UnregisterGame(it->second);
}


// 게임 서버 핸들러들을 등록합니다.
void RegisterGameEventHandlers() {
  EncodingScheme encoding = kUnknownEncoding;
  if (FLAGS_tcp_json_port || FLAGS_udp_json_port || FLAGS_http_json_port) {
    encoding = kJsonEncoding;
  }
  if (FLAGS_tcp_protobuf_port || FLAGS_udp_protobuf_port || FLAGS_http_protobuf_port) {
    if (encoding != kUnknownEncoding) {
      LOG(FATAL) << "Cannot set both JSON and Protobuf. "
                 << "Enable only one in MANIFEST.lobby.json";
    }
    encoding = kProtobufEncoding;
  }
  if (encoding == kUnknownEncoding) {
    LOG(FATAL) << "Either JSON or Protobuf must be enabled.";
  }

  HandlerRegistry::Install2(
      OnSessionOpened, bind(&OnSessionClosed, _1, _2, encoding));
  HandlerRegistry::RegisterTcpTransportDetachedHandler(
      bind(&OnTransportTcpDetached, _1, encoding));
  AccountManager::RegisterRedirectionHandler(OnClientRedirected);

  if (encoding == kJsonEncoding) {
    // JSON 인 경우 메시지 핸들러.
    HandlerRegistry::Register("ready", OnReadySignal);
    HandlerRegistry::Register("relay", OnRelayRequested);
    HandlerRegistry::Register("result", OnResultRequested);
    the_scheme = kJsonEncoding;
  } else if (encoding == kProtobufEncoding) {
    // Protobuf 인 경우 메시지 핸들러
    HandlerRegistry::Register2("ready", OnReadySignal2);
    HandlerRegistry::Register2("relay", OnRelayRequested2);
    HandlerRegistry::Register2("result", OnResultRequested2);
    the_scheme = kProtobufEncoding;
  }
}

}  // namespace pong
