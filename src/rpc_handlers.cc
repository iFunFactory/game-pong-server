#include "rpc_handlers.h"
#include "redirection_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "pong_loggers.h"
#include "pong_messages.pb.h"

DECLARE_string(app_flavor);

namespace pong_rpc {
	// 'match_result' 요청이 도착했을 때 호출될 핸들러.
	void OnMatchResultRpc(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &request) {

		BOOST_ASSERT(request->HasExtension(match_context_rpc));
		const MatchContextRpcMessage &echo = request->GetExtension(match_context_rpc);
		const OnewayMatchRpcMessage &echo_context = echo.context();

		string playerA = echo.player_a();
		string playerB = echo.player_b();
		string id = echo_context.id();
		string msg = echo_context.message();

		Ptr<Session> session = AccountManager::FindLocalSession(id);

		Json response;
		response["result"] = msg;

		if (msg.compare("Success") == 0) {
			response["A"] = playerA;
			response["B"] = playerB;

			if (playerA.compare(id) == 0)
				session->AddToContext("opponent", playerB);
			else
				session->AddToContext("opponent", playerA);

			session->AddToContext("matching", "done");
			session->AddToContext("ready", 0);

			LOG(INFO) << "[" << FLAGS_app_flavor  << "] Succeed in matchmaking. id : " << id;
			pong_redirection::MoveServerByTag(session, "game");
		}
		else {
			LOG(INFO) << "[" << FLAGS_app_flavor  << "] Fail to matchmaking. id : " << id;
			// 타임아웃이 되거나 중복 요청인 경우 실패합니다.
			session->AddToContext("matching", "failed");
		}

		session->SendMessage("match", response, kDefaultEncryption, kTcp);
	}

	void MatchmakingCallback(const string &id,
			const MatchmakingClient::Match &match,
			MatchmakingClient::MatchResult result) {

		// 로비 서버를 찾습니다.
		Rpc::PeerMap peers;
		Rpc::GetPeersWithTag(&peers, "lobby");
		Rpc::PeerId target;
		for (Rpc::PeerMap::const_iterator itr = peers.begin(); itr != peers.end(); ++itr) {
			target = itr->first;
		}

		Ptr<FunRpcMessage> request(new FunRpcMessage);
		// type 은 RegisterHandler 에 등록된 type 과 같아야합니다.
		request->set_type("match_result");
		MatchContextRpcMessage *msg = request->MutableExtension(match_context_rpc);
		OnewayMatchRpcMessage *msg_req = msg->mutable_context();

		msg_req->set_id(id);

		if (result == MatchmakingClient::kMRSuccess) {
			msg->set_player_a(match.context["A"].ToString());
			msg->set_player_b(match.context["B"].ToString());

			msg_req->set_message("Success");
		}
		else if (result == MatchmakingClient::kMRAlreadyRequested) {
			msg_req->set_message("AlreadyRequested");
			// 이미 매치메이킹 요청을 했습니다.
		}
		else if (result == MatchmakingClient::kMRTimeout) {
			msg_req->set_message("Timeout");
			// 지정된 시간안에 매치메이킹이 성사되지 않았습니다.
		}
		else {
			msg_req->set_message("Error");
		}

		Rpc::Call(target, request);
	}

	// 'match_make' 요청이 도착했을 때 호출될 핸들러.
	void OnMatchMakeRpc(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &request,
			const Rpc::ReadyBack &finisher) {

		BOOST_ASSERT(request->HasExtension(match_rpc));
		const MatchRpcMessage &echo = request->GetExtension(match_rpc);
		const OnewayMatchRpcMessage &echo_req = echo.request();

		string id = echo_req.id();
		string msg = echo_req.message();

		Json context;
		context.SetObject();
		WallClock::Duration timeout = WallClock::FromMsec(10 * 1000);

		MatchmakingClient::StartMatchmaking(0, id, context, MatchmakingCallback, MatchmakingClient::kNullProgressCallback, timeout);

		Ptr<FunRpcMessage> response(new FunRpcMessage);

		response->set_type("match_response");
		MatchRpcMessage *echo2 = response->MutableExtension(match_rpc);
		OnewayMatchRpcMessage *echo_res = echo2->mutable_response();

		echo_res->set_id(id);
		echo_res->set_message(msg);

		// OnMatchResponseRpc에 response합니다.
		finisher(response);
	}

	// 'match_make'에 대한 응답이 도착했을 때 호출될 Callback
	void MatchmakeRpcResponseCallback(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &res) {

		if (not res) {
			LOG(ERROR) << "rpc call failed";
			return;
		}

		const MatchRpcMessage &echo = res->GetExtension(match_rpc);
		const OnewayMatchRpcMessage &echo_res = echo.response();

		string id = echo_res.id();

		LOG(INFO) << "[" << FLAGS_app_flavor  << "] id : " << echo_res.id() << " response msg : " << echo_res.message() << " from " << sender;
		Ptr<Session> session = AccountManager::FindLocalSession(id);
		session->AddToContext("matching", "doing");
	}


	void MatchmakingCancelled(const string &id, MatchmakingClient::CancelResult result) {
		// 로비 서버를 찾습니다.
		Rpc::PeerMap peers;
		Rpc::GetPeersWithTag(&peers, "lobby");
		Rpc::PeerId target;

		for (Rpc::PeerMap::const_iterator itr = peers.begin(); itr != peers.end(); ++itr) {
			target = itr->first;
		}

		Ptr<FunRpcMessage> request(new FunRpcMessage);
		request->set_type("cancel_match_result");
		OnewayMatchRpcMessage *msg = request->MutableExtension(oneway_match_rpc);

		msg->set_id(id);

		if (result == MatchmakingClient::kCRNoRequest) {
                        // 매치메이킹 요청이 없었습니다. (취소할 Matchmaking 이 없습니다)
			msg->set_message("NoRequest");
                }
                else if (result == MatchmakingClient::kCRError) {
                        msg->set_message("Error");
                }
                else
                {
                        msg->set_message("Cancel");
			LOG(INFO) << "[" << FLAGS_app_flavor  << "] Matchmaking id cancelled. id : " << id;
		}

		Rpc::Call(target, request);

        }

	// 'cancel_match' 요청이 도착했을 때 호출될 핸들러
	void OnCancelMatchRpc(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &request,
			const Rpc::ReadyBack &finisher) {

		BOOST_ASSERT(request->HasExtension(match_rpc));
		const MatchRpcMessage &echo = request->GetExtension(match_rpc);
		const OnewayMatchRpcMessage &echo_req = echo.request();

		string id = echo_req.id();
		string msg = echo_req.message();

		Ptr<FunRpcMessage> response(new FunRpcMessage);
		response->set_type("match_response");
		MatchRpcMessage *echo2 = response->MutableExtension(match_rpc);
		OnewayMatchRpcMessage *echo_res = echo2->mutable_response();

		echo_res->set_id(id);
		echo_res->set_message(msg);

		finisher(response);
		// matchmaking 취소를 요청합니다.
		LOG(INFO) << "Request cancel matchmaking...";
		MatchmakingClient::CancelMatchmaking(0, id, MatchmakingCancelled);
	}

	// 'cancel_match' 응답이 도착했을 때 호출될 Callback
	void OnCancelMatchRpcReplyCallback(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &res) {

		if (not res) {
			LOG(ERROR) << "[" << FLAGS_app_flavor  << "] rpc call failed";
			return;
		}

		const MatchRpcMessage &echo = res->GetExtension(match_rpc);
		const OnewayMatchRpcMessage &echo_res = echo.response();

		string id = echo_res.id();

		Ptr<Session> session = AccountManager::FindLocalSession(id);
	        session->AddToContext("matching", "cancel");
	}

	// 'cancel_match_result' 요청이 도착했을 때 호출될 핸들러
	void OnCancelMatchResultRpc(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &request) {

		BOOST_ASSERT(request->HasExtension(oneway_match_rpc));
		const OnewayMatchRpcMessage &msg = request->GetExtension(oneway_match_rpc);

		string result = msg.message();
		string id = msg.id();

		Ptr<Session> session = AccountManager::FindLocalSession(id);

		Json response;
		response["result"] = result;
		session->SendMessage("match", response, kDefaultEncryption, kTcp);
	}


	void MatchingCancelledByTransportDetaching(const string &id, MatchmakingClient::CancelResult result) {
		LOG(INFO) << "[" << FLAGS_app_flavor << "] MatchingCancelledByTransportDetaching : " << id;
	}

	// 'cancel_match_by_tcp_detached' 요청이 도착했을 때 호출될 핸들러
	void OnCancelMatchByTcpDetached(const Rpc::PeerId &sender, const Rpc::Xid &xid,
			const Ptr<const FunRpcMessage> &request) {

		BOOST_ASSERT(request->HasExtension(oneway_match_rpc));
		const OnewayMatchRpcMessage &msg = request->GetExtension(oneway_match_rpc);

		string id = msg.id();
		MatchmakingClient::CancelMatchmaking(0, id, MatchingCancelledByTransportDetaching);
	}


	// RPC 핸들러 등록
	void RegisterRpcHandlers() {
		Rpc::RegisterHandler("match_make", OnMatchMakeRpc);
		Rpc::RegisterVoidReplyHandler("match_result", OnMatchResultRpc);

		Rpc::RegisterHandler("cancel_match", OnCancelMatchRpc);
		Rpc::RegisterVoidReplyHandler("cancel_match_result", OnCancelMatchResultRpc);
		Rpc::RegisterVoidReplyHandler("cancel_match_by_tcp_detached", OnCancelMatchByTcpDetached);
	}


	// 외부에서 RPC Call하기 위한 인터페이스들
	void MatchmakingRpc(const Ptr<Session> session) {
		string id;
		session->GetFromContext("id", &id);
		Rpc::PeerMap servers;
		Rpc::GetPeersWithTag(&servers, "matchmaker");

		for (Rpc::PeerMap::const_iterator itr = servers.begin(); itr != servers.end(); ++itr) {
			const Rpc::PeerId &target = itr->first;

			Ptr<FunRpcMessage> request(new FunRpcMessage);
			request->set_type("match_make");
			MatchRpcMessage *msg = request->MutableExtension(match_rpc);
			OnewayMatchRpcMessage *msg_req = msg->mutable_request();

			msg_req->set_id(id);
			msg_req->set_message("match make!");
			Rpc::Call(target, request, MatchmakeRpcResponseCallback);
		}
	}

	void CancelMatchmakingRpc(const Ptr<Session> session) {
		string id;
		session->GetFromContext("id", &id);

		Rpc::PeerMap servers;
		Rpc::GetPeersWithTag(&servers, "matchmaker");

		for (Rpc::PeerMap::const_iterator itr = servers.begin(); itr != servers.end(); ++itr) {
			const Rpc::PeerId &target = itr->first;

			Ptr<FunRpcMessage> request(new FunRpcMessage);
			request->set_type("cancel_match");
			MatchRpcMessage *msg = request->MutableExtension(match_rpc);
			OnewayMatchRpcMessage *msg_req = msg->mutable_request();
			msg_req->set_id(id);
			msg_req->set_message("Matchmaking is cancelled.");
			Rpc::Call(target, request, OnCancelMatchRpcReplyCallback);
		}
		session->AddToContext("matching", "cancel");
		// matchmaking 취소를 요청합니다.
	}

	void CancelMatchmakingRpcByTcpDetached(const Ptr<Session> session) {
		string id;
		session->GetFromContext("id", &id);
		Rpc::PeerMap servers;
		Rpc::GetPeersWithTag(&servers, "matchmaker");

		for (Rpc::PeerMap::const_iterator itr = servers.begin(); itr != servers.end(); ++itr) {
			const Rpc::PeerId &target = itr->first;

			Ptr<FunRpcMessage> request(new FunRpcMessage);
			request->set_type("cancel_match_by_tcp_detached");
			OnewayMatchRpcMessage *msg = request->MutableExtension(oneway_match_rpc);
			msg->set_id(id);
			msg->set_message("Matchmaking is cancelled by tcp detached.");
			Rpc::Call(target, request);
		}
	}
}
