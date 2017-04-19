#include "common_handlers.h"

#include <funapi.h>

#include "pong_loggers.h"
#include "pong_messages.pb.h"


namespace pong {

// 클라이언트를 다른 서버로 이동시킵니다.
void MoveServerByTag(const Ptr<Session> session, const string &tag) {
  // 아이디를 가져옵니다.
  string id;
  session->GetFromContext("id", &id);

  // tag 에 해당하는 서버중 하나를 무작위로 고릅니다.
  Rpc::PeerId target = PickServerRandomly(tag);
  if (target.is_nil()) {
    LOG(ERROR) << "Client redirecting failure. No target server: id=" << id
               << ", tag=" << tag;
    return;
  }

  // session context 를 JSON string 으로 저장합니다. 서버 이동 후
  // 새로운 session 으로 옮기기 위함입니다.
  string extra_data;
  {
    boost::mutex::scoped_lock lock(*session);
    extra_data = session->GetContext().ToString();
  }

  // session 을 target 서버로 이동시키며 extra_data 를 함께 전달합니다.
  if (AccountManager::RedirectClient(session, target, extra_data)) {
    LOG(INFO) << "Client redirecting: id=" << id << ", tag="
              << tag << " server";
  } else {
    // 로그인하지 않았거나 tag 에 해당하는 서버가 없으면 발생합니다.
    LOG(ERROR) << "Client redirecting failure. Not logged in or "
                  "No target server: id=" << id << ", tag=" << tag;
  }
}


// 클라이언트가 다른 서버에서 이동해 왔을 때 불립니다.
void OnClientRedirected(
    const std::string &account_id, const Ptr<Session> &session, bool success,
    const std::string &extra_data) {
  if (not success) {
    LOG(WARNING) << "Client redirecting failure. Blocked by the engine: id="
                 << account_id;
    session->Close();
    return;
  }

  // 이전 서버의 Session Context 를 적용합니다.
  Json context;
  context.FromString(extra_data);
  {
    boost::mutex::scoped_lock lock(*session);
    session->SetContext(context);
  }

  LOG(INFO) << "Client redirected: id=" << account_id;
}


// 이 서버에 로그인된 유저를 다른 서버에서 로그아웃 시킬 때 불립니다.
void OnLoggedOutRemotely(const string &id, const Ptr<Session> &session) {
  LOG(INFO) << "Close session. by logged out remotely: id=" << id;
  session->Close();
}


// 공통 핸들러를 등록합니다.
void RegisterCommonHandlers() {
  AccountManager::RegisterRedirectionHandler(OnClientRedirected);
  AccountManager::RegisterRemoteLogoutHandler(OnLoggedOutRemotely);
}

}  // namespace pong
