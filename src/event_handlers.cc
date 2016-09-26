#include "event_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "pong_loggers.h"
#include "pong_messages.pb.h"

namespace pong {

	// session opened
	void OnSessionOpened(const Ptr<Session> &session) {
		logger::SessionOpened(to_string(session->id()), WallClock::Now());
	}

	// session closed
	void OnSessionClosed(const Ptr<Session> &session, SessionCloseReason reason) {
		logger::SessionClosed(to_string(session->id()), WallClock::Now());
	}
	
	// 매치메이킹 중 클라이언트와의 연결이 끊어졌을 때의 콜백입니다.
	void MatchingCancelledByTransportDetaching(const string &id, MatchmakingClient::CancelResult result) {
		LOG(INFO) << "MatchingCancelledByTransportDetaching : " << id;
	}

	// transport detached
	void OnTransportTcpDetached(const Ptr<Session> &session) {
		LOG(INFO) << "OnTransportTcpDetached : " << to_string(session->id()) << " : " << AccountManager::FindLocalAccount(session);
		// 대전 상대가 있는지 확인합니다.
		string opponentId;
		session->GetFromContext("opponent", &opponentId);
		if (!opponentId.empty())
		{
			// 대전 상대가 있는 경우, 상대에게 승리 메세지를 보냅니다.
			Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);
			if (opponentSession && opponentSession->IsTransportAttached())
			{
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
			MatchmakingClient::CancelMatchmaking(0, AccountManager::FindLocalAccount(session), MatchingCancelledByTransportDetaching);
		}
		// 로그아웃하고 세션을 종료합니다.
		AccountManager::SetLoggedOut(AccountManager::FindLocalAccount(session));
		session->Close();
	}

	// 메세지 핸들러
	
	// 로그인 요청
	void OnAccountLogin(const Ptr<Session> &session, const Json &message) {
		// 각 플랫폼 별 device id를 사용하여 중복체크만 합니다.
		string id = message["id"].GetString();
		Json response;
		if (AccountManager::CheckAndSetLoggedIn(id, session)) {
			logger::PlayerLoggedIn(to_string(session->id()), id, WallClock::Now());
			response["result"] = "ok";
			LOG(INFO) << "login succeed : " << id;
		}
		else {
			// 로그인 실패
			response["result"] = "nop";
			LOG(WARNING) << "login failed : " << id;
			// 중복 아이디 정책: 동일한 아이디가 있다면 이전 로그인을 취소합니다.
			AccountManager::SetLoggedOut(id);
			// TODO: 이전 세션 처리가 필요합니다.
		}
		session->SendMessage("login", response);
	}
	
	// matched
	void OnMatched(const string &id, const MatchmakingClient::Match &match, MatchmakingClient::MatchResult result) {
		Ptr<Session> session = AccountManager::FindLocalSession(id);
		if (!session)
			return;
		Json response;
		if (result == MatchmakingClient::kMRSuccess) {
			// 매치메이킹이 성공했습니다.
			response["result"] = "Success";
			response["A"] = match.context["A"];
			response["B"] = match.context["B"];
			if (match.context["A"].GetString().compare(id) == 0)
				session->AddToContext("opponent", match.context["B"].GetString());
			else
				session->AddToContext("opponent", match.context["A"].GetString());
			session->AddToContext("matching", "done");
			session->AddToContext("ready", 0);
		}
		else if (result == MatchmakingClient::kMRAlreadyRequested) {
			// 이미 매치메이킹 요청을 했습니다.
			response["result"] = "AlreadyRequested";
			session->AddToContext("matching", "failed");
		}
		else if (result == MatchmakingClient::kMRTimeout) {
		  // 지정된 시간안에 매치메이킹이 성사되지 않았습니다.
			response["result"] = "Timeout";
			session->AddToContext("matching", "failed");
		}
		else {
		  // 오류가 발생 하였습니다. 로그를 참고 합니다.
			response["result"] = "Error";
			session->AddToContext("matching", "failed");
		}
		
		LOG(INFO) << "OnMatched : " + response["result"].ToString() + " : " + match.context.ToString();
		session->SendMessage("match", response, kDefaultEncryption, kTcp);
	}
	
	// matching cancelled by timeout
	void MatchingCancelledByClientTimeout(const string &id, MatchmakingClient::CancelResult result) {
		LOG(INFO) << "MatchingCancelledByClientTimeout : " + id;
		Ptr<Session> session = AccountManager::FindLocalSession(id);
		if (!session) {
			return;
		}
		Json response;
		response["result"] = "Timeout";
		session->SendMessage("match", response, kDefaultEncryption, kTcp);
	}

	// matching requested
	void OnMatchmakingRequested(const Ptr<Session> &session, const Json &message) {
		Json context;
		context.SetObject();
		session->AddToContext("matching", "doing");
		// 매치메이킹 타임아웃을 지정합니다.
		WallClock::Duration timeout = WallClock::FromMsec(10 * 1000);
		MatchmakingClient::StartMatchmaking(0, AccountManager::FindLocalAccount(session), context, OnMatched, MatchmakingClient::kNullProgressCallback, timeout);
		
		// 매치메이킹이 알 수 없는 이유로 지나치게 오래 걸리는 경우를 대비해야합니다.
		WallClock::Duration clientTimeout = timeout + WallClock::FromMsec(5 * 1000);
		Timer::ExpireAfter(clientTimeout,
			[session](const Timer::Id &timer_id, const WallClock::Value &clock) {
				if (!session->IsTransportAttached())
					return;
				string matchingState;
				session->GetFromContext("matching", &matchingState);
				// 매치메이킹이 아직 진행중인 경우
				if (matchingState == "doing") {
					// 매치메이킹을 취소합니다.
					MatchmakingClient::CancelMatchmaking(0, AccountManager::FindLocalAccount(session), MatchingCancelledByClientTimeout);
				}
			});
	}

	// 매치가 취소되면 호출됩니다.
	void OnCancelled(const string &id, MatchmakingClient::CancelResult result) {
		Ptr<Session> session = AccountManager::FindLocalSession(id);
		if (!session)
			return;
		Json response;
		if (result == MatchmakingClient::kCRNoRequest) {
			// 매치메이킹 요청이 없었습니다. (취소할 Matchmaking 이 없습니다)
			response["result"] = "NoRequest";
			session->AddToContext("matching", "failed");
		}
		else if (result == MatchmakingClient::kCRError) {
			// 오류가 발생 하였습니다. 로그를 참고 합니다.
			response["result"] = "Error";
			session->AddToContext("matching", "failed");
		}
		else
		{
			// 매치메이킹 요청이 취소되었습니다.
			response["result"] = "Cancel";
			session->AddToContext("matching", "cancelled");
		}

		// Matchmaking 요청을 취소 했습니다.
		LOG(INFO) << "match cancelled : " + id;
		// 클라이언트에 응답을 보내는 작업 등의 후속처리를 합니다.
		session->SendMessage("match", response, kDefaultEncryption, kTcp);
	}

	// 매치 취소를 요청하는 핸들러입니다.
	void OnCancelRequested(const Ptr<Session> &session, const Json &message) {
		session->AddToContext("matching", "cancel");
		// matchmaking 취소를 요청합니다.
		MatchmakingClient::CancelMatchmaking(0, AccountManager::FindLocalAccount(session), OnCancelled);
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
		if (opponentSession && opponentSession->IsTransportAttached())
			opponentSession->SendMessage("relay", message);
	}
	
	void OnResultRequested(const Ptr<Session> &session, const Json &message) {
		// 패배한 쪽만 result를 보내도록 되어있습니다.
		string opponentId;
		session->GetFromContext("opponent", &opponentId);
		Ptr<Session> opponentSession = AccountManager::FindLocalSession(opponentId);
		if (opponentSession && opponentSession->IsTransportAttached()) {
			// 상대에게 승리했음을 알립니다.
			Json winMessage;
			winMessage["result"] = "win";
			opponentSession->SendMessage("result", winMessage);
		}
		// 패배 확인 메세지를 보냅니다.
		session->SendMessage("result", message);
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
		}
	}

}  // namespace pong
