#include "common_handlers.h"

#include <funapi.h>

#include "pong_loggers.h"
#include "pong_messages.pb.h"

DECLARE_string(app_flavor);

namespace pong {

	void MoveServerByTag(const Ptr<Session> session, const string &tag) {
		string id;
		session->GetFromContext("id", &id);

    Rpc::PeerId target = PickServerRandomly(tag);
    if (target.is_nil()) {
			LOG(ERROR) << "Fail to redirect. the " <<  tag << " server is not running.";
			return;
    }

    string extra_data;
    {
      boost::mutex::scoped_lock lock(*session);
      extra_data = session->GetContext().ToString();
    }

    if (!AccountManager::RedirectClient(session,  // 옮겨갈 클라이언트 세션
          target,  // 옮겨갈 서버
          extra_data)) {  // 추가 데이터
      LOG(INFO) << "[" << FLAGS_app_flavor << "] Client redirecting is failed!" << id;
      // 해당 서버가 없거나, 로그인되지 않았거나 한 경우, 실패하며, 여기서 처리
    } else {
      LOG(INFO) << "[" << FLAGS_app_flavor << "] Redirecting message sent to " << id << " msg: move to " << tag << " server";
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

  void OnLoggedOutRemotely(const string &id, const Ptr<Session> &session) {
    // 다른 서버에서 로그아웃시켰습니다.
    LOG(INFO) << "Close session. by logged out remotely: id=" << id;
    session->Close();
  }

	void RegisterCommonHandlers() {
		AccountManager::RegisterRedirectionHandler(OnClientRedirected);
    AccountManager::RegisterRemoteLogoutHandler(OnLoggedOutRemotely);
	}
}
