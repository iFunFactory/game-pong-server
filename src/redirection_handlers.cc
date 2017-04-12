#include "redirection_handlers.h"

#include <funapi.h>

#include "pong_loggers.h"
#include "pong_messages.pb.h"

DECLARE_string(app_flavor);

namespace pong_redirection {

	void MoveServerByTag(const Ptr<Session> session, const string &tag) {
		string id;
		session->GetFromContext("id", &id);

		Rpc::PeerMap servers;
		Rpc::GetPeersWithTag(&servers, tag);

		if(servers.size() == 0) {
			string msg = "Fail to redirect. the " + tag + " server is not running.";
			LOG(INFO) << msg;
			return;
		}

		for (Rpc::PeerMap::const_iterator itr = servers.begin(); itr != servers.end(); ++itr) {
			const Rpc::PeerId &target = itr->first;

			Json sessionContext = session->GetContext();
			std::string extra_data = sessionContext.ToString();

			if (!AccountManager::RedirectClient(session,  // 옮겨갈 클라이언트 세션
						target,  // 옮겨갈 서버
						extra_data)) {  // 추가 데이터

				LOG(INFO) << "[" << FLAGS_app_flavor << "] Client redirecting is failed!" << id;
				// 해당 서버가 없거나, 로그인되지 않았거나 한 경우, 실패하며, 여기서 처리
			} else {
				LOG(INFO) << "[" << FLAGS_app_flavor << "] Redirecting message sent to " << id << " msg: move to " << tag << " server";
			}
			break;
		}
	}


        void OnClientRedirected(const std::string &account_id, // 클라이언트 아이디
                        const Ptr<Session> &session,    // 이동한 클라이언트
                        bool success,                   // 인증 성공 여부
                        const std::string &extra_data) {  // 추가 데이터

		if (success) {
			Json context;
			context.FromString(extra_data);

			session->SetContext(context);
		} else {
			LOG(INFO) << "[" << FLAGS_app_flavor << "] Fail to redirection. Session is closed.";
			session->Close();
			// 인증 실패
		}
	}

	void RegisterRedirectionHandlers() {
		AccountManager::RegisterRedirectionHandler(OnClientRedirected);
	}
}
