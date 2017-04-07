#include "event_handlers.h"
#include "leaderboard_handlers.h"
#include "redirection_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "matchmaking.h"
#include "pong_loggers.h"
#include "pong_messages.pb.h"
#include "pong_types.h"

DECLARE_string(app_flavor);

namespace pong {

Json MakeResponse(const string &result, const string &message) {
  Json response;
  response["result"] = result;
  response["message"] = message;
  return response;
}


Json MakeResponse(const string &result) {
  Json response;
  response["result"] = result;
  return response;
}


	// session opened
	void OnSessionOpened(const Ptr<Session> &session) {
		logger::SessionOpened(to_string(session->id()), WallClock::Now());
	}

	// session closed
	void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason) {
		logger::SessionClosed(to_string(session->id()), WallClock::Now());
	}

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

	void AsyncLogoutCallback(const string &id, const Ptr<Session> &session, bool ret)
	{
		if (ret == 1) {
			LOG(INFO) << "[" << FLAGS_app_flavor << "] logout succeed : " << id;
		}
		else {
			LOG(WARNING) << "[" << FLAGS_app_flavor << "] logout failed : " << id;
		}
	}

	void AsyncLoginCallback(const string &id, const Ptr<Session> &session,
                          bool success) {
    if (not success) {
      // 로그인에 실패 응답을 보냅니다. 중복, 이중 로그인 등이 원인입니다.
      LOG(INFO) << "Failed to login: id=" << id;
      session->SendMessage("login", MakeResponse("nop", "fail to login"),
                           kDefaultEncryption, kTcp);
      return;
    }

    // User Object 를 가져옵니다.
		Ptr<User> user = User::FetchById(id);

    if (not user) {
      // 새로운 유저를 생성합니다.
      User::Create(id);
      LOG(INFO) << "Registered new user: id=" << id;
    }
    LOG(INFO) << "Succeed to login: id=" << id;

    // 로그인 Activitiy Log 를 남깁니다.
	  logger::PlayerLoggedIn(to_string(session->id()), id, WallClock::Now());

    // Session 에 Login 한 ID 를 저장합니다.
    session->AddToContext("id", id);

    // 응답을 보냅니다.
		Json response = MakeResponse("ok");
		response["id"] = id;
		response["winCount"] = user->GetWinCount();
		response["loseCount"] = user->GetLoseCount();
		response["curRecord"] = pong_lb::GetCurrentRecordById(id);

   	session->SendMessage("login", response, kDefaultEncryption, kTcp);
	}

	void OnFacebookAuthenticated(
		const string &fb_uid, const Ptr<Session> &session,
		const AccountAuthenticationRequest &request,
		const AccountAuthenticationResponse &response, const bool &error) {
    if (error) {
      // 인증에 오류가 있습니다. 장애 오류입니다.
      LOG(ERROR) << "Failed to authenticate. Facebook authentication error: "
                 << "id=" << fb_uid;
      session->SendMessage("login",
                           MakeResponse("nop", "facebook authentication error"),
                           kDefaultEncryption, kTcp);
      return;
    }

    if (not response.success) {
      // 인증에 실패했습니다. 올바르지 않은 access token 입니다.
      LOG(INFO) << "Failed to authenticate. Wrong Facebook access token: "
                << "id=" << fb_uid;
      string fail_message = "facebook authentication failed: " +
                            response.reason_description;
      session->SendMessage("login", MakeResponse("nop", fail_message),
                           kDefaultEncryption, kTcp);
      return;
    }

		// 인증에 성공했습니다.
    LOG(INFO) << "Succeed to authenticate facebook account: id=" << fb_uid;

    // 이어서 로그인 처리를 진행합니다.
		AccountManager::CheckAndSetLoggedInAsync(fb_uid, session,
                                             AsyncLoginCallback);
	}

	// transport detached
	void OnTransportTcpDetached(const Ptr<Session> &session) {
		LOG(INFO) << "[" << FLAGS_app_flavor << "] OnTransportTcpDetached : " << to_string(session->id()) << " : " << AccountManager::FindLocalAccount(session);
		// 대전 상대가 있는지 확인합니다.
		string opponentId;
		session->GetFromContext("opponent", &opponentId);
		if (!opponentId.empty())
		{
			// 대전 상대가 있는 경우, 상대에게 승리 메세지를 보냅니다.
			Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);
			if (opponentSession && opponentSession->IsTransportAttached())
			{
				string myId;
				session->GetFromContext("id", &myId);
				UpdateMatchRecord(opponentId, myId);
				pong_lb::IncreaseCurWincount(opponentId);
				pong_lb::ResetCurWincount(myId);
				pong_redirection::MoveServerByTag(opponentSession, "lobby");

				Json message;
				message["result"] = "win";
				opponentSession->SendMessage("result", message, kDefaultEncryption, kTcp);
			}
		}
		// 매치메이킹이 진행중인지 확인합니다.
		string matchingContext;
		session->GetFromContext("matching", &matchingContext);
		if (!matchingContext.empty() && matchingContext == "doing")
		{
			// 매치메이킹이 진행 중인 경우, 취소합니다.
      string id;
      session->GetFromContext("id", &id);
      // Matchmaking cancel 결과를 처리할 람다 함수입니다.
      auto cancel_cb = [](const string &player_id,
                          MatchmakingClient::CancelResult result) {
        if (result == MatchmakingClient::kCRSuccess) {
          LOG(INFO) << "Succeed to cancel matchmaking by TCP disconnecting: "
                    << player_id;
        } else {
          LOG(INFO) << "Failed to cancel matchmaking by TCP disconnecting: "
                    << player_id;
        }
      };

      MatchmakingClient::CancelMatchmaking(kMatch1vs1, id, cancel_cb);
		}

		// 로그아웃하고 세션을 종료합니다.
		AccountManager::SetLoggedOut(AccountManager::FindLocalAccount(session));
	}

	// 메세지 핸들러

	// 로그인 요청
	void OnAccountLogin(const Ptr<Session> &session, const Json &message) {
		string id = message["id"].GetString();
		string type = message["type"].GetString();

		if(type == "fb") {
      // Facebook 인증을 먼저 합니다.
			string access_token = message["access_token"].GetString();
      AccountAuthenticationRequest request(
          "Facebook", id, MakeFacebookAuthenticationKey(access_token));
      Authenticate(request,
                   bind(&OnFacebookAuthenticated, id, session, _1, _2, _3));
		} else {
      // Guest 는 별도의 인증 없이 로그인 합니다.
			AccountManager::CheckAndSetLoggedInAsync(id, session, AsyncLoginCallback);
		}
	}

	// 매치 메이킹 취소 요청을 수행합니다.
	void OnCancelRequested(const Ptr<Session> &session, const Json &message) {
    // 로그인 한 Id 를 가져옵니다.
    string id;
    if (not session->GetFromContext("id", &id)) {
      LOG(WARNING) << "Failed to request matchmaking. Not logged in.";
      session->SendMessage("error", MakeResponse("fail", "not logged in"));
      return;
    }

    session->AddToContext("matching", "cancel");

    // Matchmaking cancel 결과를 처리할 람다 함수입니다.
    auto cancel_cb = [session](const string &player_id,
                               MatchmakingClient::CancelResult result) {
      Json response;

      if (result == MatchmakingClient::kCRSuccess) {
        LOG(INFO) << "Succeed to cancel matchmaking: id=" << player_id;
        response = MakeResponse("Cancel");
      } else if (result == MatchmakingClient::kCRNoRequest) {
        response = MakeResponse("NoRequest");
      } else {
        response = MakeResponse("Error");
      }

      session->SendMessage("match", response, kDefaultEncryption, kTcp);
    };

    // Matchmaking 취소를 요청합니다.
    MatchmakingClient::CancelMatchmaking(kMatch1vs1, id, cancel_cb);
	}

	// 매치 메이킹 요청을 수행합니다.
  void OnMatchmakingRequested(const Ptr<Session> &session, const Json &message) {
    static const WallClock::Duration kTimeout = WallClock::FromSec(10);

    // 로그인 한 Id 를 가져옵니다.
    string id;
    if (not session->GetFromContext("id", &id)) {
      LOG(WARNING) << "Failed to request matchmaking. Not logged in.";
      session->SendMessage("error", MakeResponse("fail", "not logged in"));
      return;
    }

    // Matchmaking 결과를 처리할 람다 함수입니다.
    auto match_cb = [session](const string &player_id,
                              const MatchmakingClient::Match &match,
                              MatchmakingClient::MatchResult result) {
      Json response;

      if (result == MatchmakingClient::kMRSuccess) {
        // Matchmaking 에 성공했습니다.
        LOG(INFO) << "Succeed in matchmaking: id=" << player_id;

        BOOST_ASSERT(HasJsonStringAttribute(match.context, "A"));
        BOOST_ASSERT(HasJsonStringAttribute(match.context, "B"));

        const string player_a_id = match.context["A"].GetString();
        const string player_b_id = match.context["B"].GetString();

        string opponent_id = match.context["A"].GetString();
        if (opponent_id == player_id) {
          opponent_id = match.context["B"].GetString();
        }

        response = MakeResponse("Success");
        response["A"] = player_a_id;
        response["B"] = player_b_id;

        if (player_id == player_a_id) {
          session->AddToContext("opponent", player_b_id);
        } else {
          session->AddToContext("opponent", player_a_id);
        }
        session->AddToContext("matching", "done");
        session->AddToContext("ready", 0);

        // 유저를 Game 서버로 보냅니다.
        pong_redirection::MoveServerByTag(session, "game");
      } else if (result == MatchmakingClient::kMRAlreadyRequested) {
        // Matchmaking 요청을 중복으로 보냈습니다.
        LOG(INFO) << "Failed in matchmaking. Already requested: id="
                  << player_id;
			  session->AddToContext("matching", "failed");
        response = MakeResponse("AlreadyRequested");
      } else if (result == MatchmakingClient::kMRTimeout) {
        // Matchmaking 처리가 시간 초과되었습니다.
        LOG(INFO) << "Failed in matchmaking. Timeout: id=" << player_id;
			  session->AddToContext("matching", "failed");
        response = MakeResponse("Timeout");
      } else {
        // Matchmaking 에 오류가 발생했습니다.
        LOG(ERROR) << "Failed in matchmaking. Erorr: id=" << player_id;
			  session->AddToContext("matching", "failed");
        response = MakeResponse("Error");
      }

      session->SendMessage("match", response, kDefaultEncryption, kTcp);
    };

    Json empty_player_ctxt;
    empty_player_ctxt.SetObject();

    // Matchmaking 을 요청합니다.
    MatchmakingClient::StartMatchmaking(
        kMatch1vs1, id, empty_player_ctxt, match_cb,
        // 아래 인자는 experimental 버전이 필요합니다.(2017.04.06 기준)
        // MatchmakingClient::kMostNumberOfPlayers,
        MatchmakingClient::kNullProgressCallback, kTimeout);
  }

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
	}

	void OnRanklistRequested(const Ptr<Session> &session, const Json &message) {
		pong_lb::GetTopEightList(session);
	}

	// regist handlers
	void RegisterEventHandlers() {
		{
			HandlerRegistry::Install2(OnSessionOpened, OnSessionClosed);
			HandlerRegistry::RegisterTcpTransportDetachedHandler(OnTransportTcpDetached);
		}

		{
			JsonSchema login_msg(JsonSchema::kObject, JsonSchema("id", JsonSchema::kString, true));
			HandlerRegistry::Register("login", OnAccountLogin, login_msg);
			HandlerRegistry::Register("match", OnMatchmakingRequested);
			HandlerRegistry::Register("ready", OnReadySignal);
			HandlerRegistry::Register("relay", OnRelayRequested);
			HandlerRegistry::Register("result", OnResultRequested);
			HandlerRegistry::Register("cancelmatch", OnCancelRequested);
			HandlerRegistry::Register("ranklist", OnRanklistRequested);
		}

		{
			pong_redirection::RegisterRedirectionHandlers();
		}
	}
}  // namespace pong
