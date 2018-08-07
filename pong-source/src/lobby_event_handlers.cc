#include "lobby_event_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "common_handlers.h"
#include "leaderboard.h"
#include "matchmaking.h"
#include "pong_loggers.h"
#include "pong_types.h"

#include "pong_messages.pb.h"

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

void FreeUser(const Ptr<Session> &session, EncodingScheme encoding);


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
  // 세션을 초기과 합니다.
  FreeUser(session, encoding);
}


// TCP 연결이 끊기면 불립니다.
void OnTransportTcpDetached(const Ptr<Session> &session,
                            EncodingScheme encoding) {
  string id;
  session->GetFromContext("id", &id);
  LOG_IF(INFO, not id.empty()) << "TCP disconnected: id=" << id;
  // 세션을 초기과 합니다.
  FreeUser(session, encoding);
}


// 세션을 정리합니다.
void FreeUser(const Ptr<Session> &session, EncodingScheme encoding) {
  // 유저를 정리하기 위한 Context 를 읽어옵니다.
  string matching_state;
  string id;
  session->GetFromContext("matching", &matching_state);
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

  // 매치메이킹이 진행 중이면 취소합니다.
  if (matching_state == "doing") {
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

}  // unnamed namespace


// AccountManager 의 로그인 처리가 끝나면 불립니다.
void OnLoggedIn(const string &id, const Ptr<Session> &session, bool success,
                EncodingScheme encoding) {
  if (not success) {
    // 로그인에 실패 응답을 보냅니다. 중복 로그인이 원인입니다.
    // (1. 같은 ID 로 이미 다른 Session 이 로그인 했거나,
    //  2. 이 Session 이 이미 로그인 되어 있는 경우)
    LOG(INFO) << "Failed to login: id=" << id;

    if (encoding == kJsonEncoding) {
      session->SendMessage("login", MakeResponse("nop", "fail to login"),
                           kDefaultEncryption);
    } else {
      Ptr<FunMessage> response(new FunMessage);
      LobbyLoginReply *login_response = response->MutableExtension(lobby_login_repl);
      login_response->set_result("nop");
      login_response->set_msg("fail to login");
      session->SendMessage("login", response, kDefaultEncryption);
    }

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
    user = User::Create(id);
    LOG(INFO) << "Registered new user: id=" << id;
  }
  LOG(INFO) << "Succeed to login: id=" << id;

  // 로그인 Activitiy Log 를 남깁니다.
  logger::PlayerLoggedIn(to_string(session->id()), id, WallClock::Now());

  // Session 에 Login 한 ID 를 저장합니다.
  session->AddToContext("id", id);

  // 응답을 보냅니다.
  if (encoding == kJsonEncoding) {
    Json response = MakeResponse("ok");
    response["id"] = id;
    response["winCount"] = user->GetWinCount();
    response["loseCount"] = user->GetLoseCount();
    response["curRecord"] = GetCurrentRecordById(id);

    session->SendMessage("login", response, kDefaultEncryption);
  } else {
    Ptr<FunMessage> response(new FunMessage);
    LobbyLoginReply *login_response = response->MutableExtension(lobby_login_repl);
    login_response->set_result("ok");
    login_response->set_id(id);
    login_response->set_win_count(user->GetWinCount());
    login_response->set_lose_count(user->GetLoseCount());
    login_response->set_cur_record(GetCurrentRecordById(id));

    session->SendMessage("login", response, kDefaultEncryption);
  }
}


// Facebook 인증 처리 후 불립니다.
void OnFacebookAuthenticated(
  const string &fb_uid, const Ptr<Session> &session,
  const AccountAuthenticationRequest &request,
  const AccountAuthenticationResponse &response, const bool &error,
  EncodingScheme encoding) {
  if (error) {
    // 인증에 오류가 있습니다. 장애 오류입니다.
    LOG(ERROR) << "Failed to authenticate. Facebook authentication error: "
               << "id=" << fb_uid;

    if (encoding == kJsonEncoding) {
      session->SendMessage("login",
                           MakeResponse("nop", "facebook authentication error"),
                           kDefaultEncryption);
    } else {
      Ptr<FunMessage> response(new FunMessage);
      LobbyLoginReply *login_response = response->MutableExtension(lobby_login_repl);
      login_response->set_result("nop");
      login_response->set_msg("facebook authentication error");

      session->SendMessage("login", response, kDefaultEncryption);
    }
    return;
  }

  if (not response.success) {
    // 인증에 실패했습니다. 올바르지 않은 access token 입니다.
    LOG(INFO) << "Failed to authenticate. Wrong Facebook access token: "
              << "id=" << fb_uid;
    string fail_message = "facebook authentication failed: " +
                          response.reason_description;

    if (encoding == kJsonEncoding) {
      session->SendMessage("login", MakeResponse("nop", fail_message),
                           kDefaultEncryption);
    } else {
      Ptr<FunMessage> response(new FunMessage);
      LobbyLoginReply *login_response = response->MutableExtension(lobby_login_repl);
      login_response->set_result("nop");
      login_response->set_msg(fail_message);

      session->SendMessage("login", response, kDefaultEncryption);
    }
    return;
  }

  // 인증에 성공했습니다.
  LOG(INFO) << "Succeed to authenticate facebook account: id=" << fb_uid;

  // 이어서 로그인 처리를 진행합니다.
  AccountManager::CheckAndSetLoggedInAsync(
      fb_uid, session, bind(&OnLoggedIn, _1, _2, _3, encoding));
}


void StartMatchmaking(const Ptr<Session> &session, EncodingScheme encoding) {
  // Matchmaking 최대 대기 시간은 10 초입니다.
  static const WallClock::Duration kTimeout = WallClock::FromSec(10);

  // 로그인 한 Id 를 가져옵니다.
  string id;
  if (not session->GetFromContext("id", &id)) {
    LOG(WARNING) << "Failed to request matchmaking. Not logged in.";

    if (encoding == kJsonEncoding) {
      session->SendMessage("error", MakeResponse("fail", "not logged in"));
    } else {
      Ptr<FunMessage> response(new FunMessage);
      PongErrorMessage *error = response->MutableExtension(pong_error);
      error->set_result("fail");
      error->set_msg("not logged in");
      session->SendMessage("error", response);
    }
    return;
  }

  // Matchmaking 결과를 처리할 람다 함수입니다.
  auto match_cb = [session, encoding](const string &player_id,
                                      const MatchmakingClient::Match &match,
                                      MatchmakingClient::MatchResult result) {
    Json json_response;
    Ptr<FunMessage> pbuf_response(new FunMessage);
    LobbyMatchReply *pbuf_match_reply
        = pbuf_response->MutableExtension(lobby_match_repl);

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

      if (encoding == kJsonEncoding) {
        json_response = MakeResponse("Success");
        json_response["A"] = player_a_id;
        json_response["B"] = player_b_id;
      } else {
        pbuf_match_reply->set_result("Success");
        pbuf_match_reply->set_player1(player_a_id);
        pbuf_match_reply->set_player2(player_b_id);
      }

      if (player_id == player_a_id) {
        session->AddToContext("opponent", player_b_id);
      } else {
        session->AddToContext("opponent", player_a_id);
      }
      session->AddToContext("matching", "done");
      session->AddToContext("ready", 0);

      // 유저를 Game 서버로 보냅니다.
      MoveServerByTag(session, "game");
      FreeUser(session, encoding);
    } else if (result == MatchmakingClient::kMRAlreadyRequested) {
      // Matchmaking 요청을 중복으로 보냈습니다.
      LOG(INFO) << "Failed in matchmaking. Already requested: id="
                << player_id;
      session->AddToContext("matching", "failed");

      if (encoding == kJsonEncoding) {
        json_response = MakeResponse("AlreadyRequested");
      } else {
        pbuf_match_reply->set_result("AlreadyRequested");
      }
    } else if (result == MatchmakingClient::kMRTimeout) {
      // Matchmaking 처리가 시간 초과되었습니다.
      LOG(INFO) << "Failed in matchmaking. Timeout: id=" << player_id;
      session->AddToContext("matching", "failed");

      if (encoding == kJsonEncoding) {
        json_response = MakeResponse("Timeout");
      } else {
        pbuf_match_reply->set_result("Timeout");
      }
    } else {
      // Matchmaking 에 오류가 발생했습니다.
      LOG(ERROR) << "Failed in matchmaking. Erorr: id=" << player_id;
      session->AddToContext("matching", "failed");

      if (encoding == kJsonEncoding) {
        json_response = MakeResponse("Error");
      } else {
        pbuf_match_reply->set_result("Error");
      }
    }

    if (encoding == kJsonEncoding) {
      session->SendMessage("match", json_response, kDefaultEncryption);
    } else {
      session->SendMessage("match", pbuf_response, kDefaultEncryption);
    }
  };

  // 빈 Player Context 를 만듭니다. 지금 구현에서는 Matchmaking 서버가
  // 조건 없이 Matching 합니다. Level 등의 조건으로 Matching 하려면
  // 여기에 Level 등의 Matching 에 필요한 정보를 넣습니다.
  Json empty_player_ctxt;
  empty_player_ctxt.SetObject();

  // Matchmaking 을 요청합니다.
  MatchmakingClient::StartMatchmaking(
      kMatch1vs1, id, empty_player_ctxt, match_cb,
      MatchmakingClient::kMostNumberOfPlayers,
      MatchmakingClient::kNullProgressCallback, kTimeout);
}


void CancelMatchmaking(const Ptr<Session>& session, EncodingScheme encoding) {
  // 로그인 한 Id 를 가져옵니다.
  string id;
  if (not session->GetFromContext("id", &id)) {
    LOG(WARNING) << "Failed to request matchmaking. Not logged in.";
    session->SendMessage("error", MakeResponse("fail", "not logged in"));
    return;
  }

  // 매치메이킹 취소 상태로 변경합니다.
  session->AddToContext("matching", "cancel");

  // Matchmaking cancel 결과를 처리할 람다 함수입니다.
  auto cancel_cb = [session, encoding](const string &player_id,
                                       MatchmakingClient::CancelResult result) {
    if (encoding == kJsonEncoding) {
      Json response;

      if (result == MatchmakingClient::kCRSuccess) {
        LOG(INFO) << "Succeed to cancel matchmaking: id=" << player_id;
        response = MakeResponse("Cancel");
      } else if (result == MatchmakingClient::kCRNoRequest) {
        response = MakeResponse("NoRequest");
      } else {
        response = MakeResponse("Error");
      }

      session->SendMessage("match", response, kDefaultEncryption);
    } else {
      Ptr<FunMessage> pbuf_response(new FunMessage);
      LobbyMatchReply *pbuf_match_reply
          = pbuf_response->MutableExtension(lobby_match_repl);

      if (result == MatchmakingClient::kCRSuccess) {
        LOG(INFO) << "Succeed to cancel matchmaking: id=" << player_id;
        pbuf_match_reply->set_result("Cancel");
      } else if (result == MatchmakingClient::kCRNoRequest) {
        pbuf_match_reply->set_result("NoRequest");
      } else {
        pbuf_match_reply->set_result("Error");
      }

      session->SendMessage("match", pbuf_response, kDefaultEncryption);
    }
  };

  // Matchmaking 취소를 요청합니다.
  MatchmakingClient::CancelMatchmaking(kMatch1vs1, id, cancel_cb);
}


////////////////////////////////////////////////////////////////////////////////
//
// JSON 메시지 핸들러들
//
////////////////////////////////////////////////////////////////////////////////

// 로그인 메시지를 받으면 불립니다.
void OnAccountLogin(const Ptr<Session> &session, const Json &message) {
  string id = message["id"].GetString();
  string type = message["type"].GetString();

  if(type == "fb") {
    // Facebook 인증을 먼저 합니다.
    string access_token = message["access_token"].GetString();
    AccountAuthenticationRequest request(
        "Facebook", id, MakeFacebookAuthenticationKey(access_token));
    Authenticate(request,
                 bind(&OnFacebookAuthenticated, id, session, _1, _2, _3,
                      kJsonEncoding));
  } else {
    // Guest 는 별도의 인증 없이 로그인 합니다.
    AccountManager::CheckAndSetLoggedInAsync(
        id, session, bind(&OnLoggedIn, _1, _2, _3, kJsonEncoding));
  }
}


// 매치 메이킹 요청을 수행합니다.
void OnMatchmaking(const Ptr<Session> &session, const Json &/*message*/) {
  // 실제 matchmaking 구현을 호출한다.
  StartMatchmaking(session, kJsonEncoding);
}


// 매치메이킹 취소 메시지를 받으면 불립니다.
void OnCancelMatchmaking(const Ptr<Session> &session, const Json &/*message*/) {
  CancelMatchmaking(session, kJsonEncoding);
}


// TOP 8 랭킹 메시지를 받으면 불립니다.
void OnRanklistRequested(const Ptr<Session> &session, const Json &message) {
  GetAndSendTopEightList(session, kJsonEncoding);
}


////////////////////////////////////////////////////////////////////////////////
//
// Protobuf 메시지 핸들러들
//
////////////////////////////////////////////////////////////////////////////////

void OnAccountLogin2(
    const Ptr<Session> &session, const Ptr<FunMessage> &message) {
  // pong_messages.proto 에 정의된 바에 의하면 아래 사항은 일어날 수 없습니다.
  BOOST_ASSERT(message->HasExtension(lobby_login_req));

  const LobbyLoginRequest &req
      = message->GetExtension(lobby_login_req);
  string id = req.id();
  string type = req.type();

  if(type == "fb") {
    // Facebook 인증을 먼저 합니다.
    if (not req.has_access_token()) {
      LOG(ERROR) << "FB login without an access token!";
      return;
    }
    string access_token = req.access_token();
    AccountAuthenticationRequest request(
        "Facebook", id, MakeFacebookAuthenticationKey(access_token));
    Authenticate(request,
                 bind(&OnFacebookAuthenticated, id, session, _1, _2, _3,
                      kProtobufEncoding));
  } else {
    // Guest 는 별도의 인증 없이 로그인 합니다.
    AccountManager::CheckAndSetLoggedInAsync(
        id, session, bind(&OnLoggedIn, _1, _2, _3, kProtobufEncoding));
  }
}


void OnMatchmaking2(
    const Ptr<Session> &session, const Ptr<FunMessage> &/*message*/) {
  StartMatchmaking(session, kProtobufEncoding);
}


void OnCancelMatchmaking2(
    const Ptr<Session> &session, const Ptr<FunMessage> &/*message*/) {
  CancelMatchmaking(session, kProtobufEncoding);
}


void OnRankListRequested2(
    const Ptr<Session> &session, const Ptr<FunMessage> &message) {
  GetAndSendTopEightList(session, kProtobufEncoding);
}


// 로비 서버 핸들러들을 등록합니다.
void RegisterLobbyEventHandlers() {
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

  if (encoding == kJsonEncoding) {
    // JSON 버전 Login 핸들러
    JsonSchema login_msg(JsonSchema::kObject,
                         JsonSchema("id", JsonSchema::kString, true));
    HandlerRegistry::Register("login", OnAccountLogin, login_msg);

    // JSON 버전 Matchmaking 핸들러
    HandlerRegistry::Register("match", OnMatchmaking);
    HandlerRegistry::Register("cancelmatch", OnCancelMatchmaking);

    // JSON 버전 Leaderboard 핸들러
    HandlerRegistry::Register("ranklist", OnRanklistRequested);
  } else {
    // Protobuf 버전 Login 핸들러
    HandlerRegistry::Register2("login", OnAccountLogin2);

    // Protobuf 버전 Matchmkaing 핸들러
    HandlerRegistry::Register2("match", OnMatchmaking2);
    HandlerRegistry::Register2("cancelmatch", OnCancelMatchmaking2);

    // Protobuf 버전 Leaderboard 핸들러
    HandlerRegistry::Register2("ranklist", OnRankListRequested2);
  }
}

}  // namespace pong
