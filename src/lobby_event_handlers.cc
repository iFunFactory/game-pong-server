#include "lobby_event_handlers.h"

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

void FreeUser(const Ptr<Session> &session) {
  // 유저를 정리하기 위한 Context 를 읽어옵니다.
  string matchingContext;
  string id;
  session->GetFromContext("matching", &matchingContext);
  session->GetFromContext("id", &id);

  // Session Context 를 초기화 합니다.
  session->SetContext(Json());

  // 로그아웃하고 세션을 종료합니다.
  if (not id.empty()) {
    auto logout_cb = [](const string &id, const Ptr<Session> &session,
                        bool success) {
      if (success) {
        LOG(INFO) << "Logged out(local) by session close: id=" << id;
      }
    };
    AccountManager::SetLoggedOutAsync(id, logout_cb);
  }

  // 매치메이킹이 진행중인지 확인합니다.
  if (!matchingContext.empty() && matchingContext == "doing")
  {
    // 매치메이킹이 진행 중인 경우, 취소합니다.
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
}  // unnamed namespace


	void AsyncLoginCallback(const string &id, const Ptr<Session> &session,
                          bool success) {
    if (not success) {
      // 로그인에 실패 응답을 보냅니다. 중복 로그인이 원인입니다.
      // (1. 같은 ID 로 이미 다른 Session 이 로그인 했거나,
      //  2. 이 Session 이 이미 로그인 되어 있는 경우)
      LOG(INFO) << "Failed to login: id=" << id;
      session->SendMessage("login", MakeResponse("nop", "fail to login"),
                           kDefaultEncryption, kTcp);

      // 아래 로그아웃 처리를 한 후 자동으로 로그인 시킬 수 있지만
      // 일단 클라이언트에서 다시 시도하도록 합니다.

      // 1. 이 ID 의 로그인을 풀어버립니다.(로그아웃)
      auto logout_cb = [](const string &id, const Ptr<Session> &session,
                          bool success) {
        if (success) {
          if (session) {
            // 같은 서버에 로그인 되어 있었습니다.
            LOG(INFO) << "Logged out(local) by duplicated login request: "
                      << "id=" << id;
            session->Close();
          } else {
            // 다른 서버에 로그인 되어 있었습니다.
            // 해당 서버의 OnLoggedOutRemotely() 에서 처리합니다.
            LOG(INFO) << "Logged out(remote) by duplicated login request: "
                      << "id=" << id;
          }
        }
      };
      AccountManager::SetLoggedOutGlobalAsync(id, logout_cb);

      // 2. 이 Session 의 로그인을 풀어버립니다.(로그아웃)
      string id_logged_in = AccountManager::FindLocalAccount(session);
      if (not id_logged_in.empty()) {
        // OnSessionClosed 에서 처리합니다.
        LOG(INFO) << "Close session. by duplicated login request: id=" << id;
        session->Close();
      }

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
        FreeUser(session);
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

	void OnRanklistRequested(const Ptr<Session> &session, const Json &message) {
		pong_lb::GetTopEightList(session);
	}

	// regist handlers
	void RegisterLobbyEventHandlers() {
		{
			HandlerRegistry::Install2(OnSessionOpened, OnSessionClosed);
			HandlerRegistry::RegisterTcpTransportDetachedHandler(OnTransportTcpDetached);
		}

		{
			JsonSchema login_msg(JsonSchema::kObject, JsonSchema("id", JsonSchema::kString, true));
			HandlerRegistry::Register("login", OnAccountLogin, login_msg);
			HandlerRegistry::Register("match", OnMatchmakingRequested);
			HandlerRegistry::Register("cancelmatch", OnCancelRequested);
			HandlerRegistry::Register("ranklist", OnRanklistRequested);
		}

    {
      AccountManager::RegisterRemoteLogoutHandler(OnLoggedOutRemotely);
    }

		{
			pong_redirection::RegisterRedirectionHandlers();
		}
	}
}  // namespace pong