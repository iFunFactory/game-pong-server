#include "game_event_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "leaderboard_handlers.h"
#include "matchmaking.h"
#include "pong_loggers.h"
#include "pong_types.h"
#include "redirection_handlers.h"

DECLARE_string(app_flavor);

namespace pong {

namespace {

	void UpdateMatchRecord(const string& winnerId, const string& loserId)
	{
		Ptr<User> winner = User::FetchById(winnerId);
		Ptr<User> loser = User::FetchById(loserId);

		if(not winner) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] Cannot find winner's id in db" << winnerId;
			return;
		}

		if(not loser) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] Cannot find loser's id in db" << loserId;
			return;
		}

		winner->SetWinCount(winner->GetWinCount() + 1);
		loser->SetLoseCount(loser->GetLoseCount() + 1);
	}


void FreeUser(const Ptr<Session> &session) {
  // 유저를 정리하기 위한 Context 를 읽어옵니다.
  string opponentId;
  string myId;
  session->GetFromContext("opponent", &opponentId);
  session->GetFromContext("id", &myId);

  // Session Context 를 초기화 합니다.
  session->SetContext(Json());

  // 로그아웃하고 세션을 종료합니다.
  if (not myId.empty()) {
    auto logout_cb = [](const string &id, const Ptr<Session> &session,
                        bool success) {
      if (success) {
        LOG(INFO) << "Logged out(local) by session close: id=" << id;
      }
    };
    AccountManager::SetLoggedOutAsync(myId, logout_cb);
  }

  // 대전 상대가 있는 경우, 상대에게 승리 메세지를 보냅니다.
  if (opponentId.empty()) {
    return;
  }
  Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);
  if (not opponentSession || not opponentSession->IsTransportAttached()) {
    return;
  }

  Event::Invoke(bind(&UpdateMatchRecord, opponentId, myId));
  pong_lb::IncreaseCurWincount(opponentId);
  pong_lb::ResetCurWincount(myId);
  pong_redirection::MoveServerByTag(opponentSession, "lobby");

  Json message;
  message["result"] = "win";
  opponentSession->SendMessage("result", message, kDefaultEncryption, kTcp);

  opponentSession->DeleteFromContext("opponent");
  FreeUser(opponentSession);
}

	// session opened
	void OnSessionOpened(const Ptr<Session> &session) {
		logger::SessionOpened(to_string(session->id()), WallClock::Now());
	}

	// session closed
	void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason) {
		logger::SessionClosed(to_string(session->id()), WallClock::Now());
    FreeUser(session);
	}

	// transport detached
	void OnTransportTcpDetached(const Ptr<Session> &session) {
    LOG(INFO) << "TCP disconnected: id="
              << AccountManager::FindLocalAccount(session);
    FreeUser(session);
	}

  void OnLoggedOutRemotely(const string &id, const Ptr<Session> &session) {
    // 다른 서버에서 로그아웃시켰습니다.
    LOG(INFO) << "Close session. by logged out remotely: id=" << id;
    session->Close();
  }
}  // unnamed namesapce

	// 메세지 핸들러

	// 매칭 성공 후, 게임을 플레이할 준비가 되면 클라이언트는 ready를 보냅니다.
	void OnReadySignal(const Ptr<Session> &session, const Json &message) {
		session->AddToContext("ready", 1);
		string opponentId;
		session->GetFromContext("opponent", &opponentId);
		Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);
		// 상대의 상태를 확인합니다.
		if (opponentSession && opponentSession->IsTransportAttached()) {
			int64_t is_opponent_ready = 0;
			opponentSession->GetFromContext("ready", &is_opponent_ready);
			if (is_opponent_ready == 1) {
				// 둘 다 준비가 되었습니다. 시작 신호를 보냅니다.
				Json response;
				response["result"] = "ok";
				session->SendMessage("start", response);
				opponentSession->SendMessage("start", response);
			}
		}
		else {
			// 상대가 접속을 종료했습니다.
			Json response;
			response["result"] = "opponent disconnected";
			session->SendMessage("match", response, kDefaultEncryption, kTcp);
			return;
		}
	}

	// 메세지 릴레이를 수행합니다. tcp, udp 둘 다 이 함수로 처리합니다.
	void OnRelayRequested(const Ptr<Session> &session, const Json &message) {
		string opponentId;
		session->GetFromContext("opponent", &opponentId);
		Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);
		if (opponentSession && opponentSession->IsTransportAttached()) {

			LOG(INFO) << "[" << FLAGS_app_flavor << "] relay. Session : " << session;
			opponentSession->SendMessage("relay", message);
		}
	}

	void OnResultRequested(const Ptr<Session> &session, const Json &message) {
		// 패배한 쪽만 result를 보내도록 되어있습니다.
		string myId;
		session->GetFromContext("id", &myId);

		string opponentId;
		session->GetFromContext("opponent", &opponentId);
		Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);

		UpdateMatchRecord(opponentId, myId);

		if (opponentSession && opponentSession->IsTransportAttached()) {
			// 상대에게 승리했음을 알립니다.
			Json winMessage;
			winMessage["result"] = "win";
			opponentSession->SendMessage("result", winMessage);
			pong_lb::IncreaseCurWincount(opponentId);
		}
		// 패배 확인 메세지를 보냅니다.
		session->SendMessage("result", message);
		pong_lb::ResetCurWincount(myId);

		// 각각 상대방에 대한 정보를 삭제합니다.
		opponentSession->DeleteFromContext("opponent");
		session->DeleteFromContext("opponent");

		// 두 플레이어를 lobby서버로 이동시킵니다.
		pong_redirection::MoveServerByTag(opponentSession, "lobby");
		pong_redirection::MoveServerByTag(session, "lobby");

    FreeUser(opponentSession);
    FreeUser(session);
	}

	// regist handlers
	void RegisterGameEventHandlers() {
		{
			HandlerRegistry::Install2(OnSessionOpened, OnSessionClosed);
			HandlerRegistry::RegisterTcpTransportDetachedHandler(OnTransportTcpDetached);
		}

		{
			HandlerRegistry::Register("ready", OnReadySignal);
			HandlerRegistry::Register("relay", OnRelayRequested);
			HandlerRegistry::Register("result", OnResultRequested);
		}

    {
      AccountManager::RegisterRemoteLogoutHandler(OnLoggedOutRemotely);
    }

		{
			pong_redirection::RegisterRedirectionHandlers();
		}
	}
}  // namespace pong
