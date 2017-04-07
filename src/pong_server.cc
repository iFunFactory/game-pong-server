#include <boost/bind.hpp>
#include <funapi.h>
#include <gflags/gflags.h>

#include "event_handlers.h"
#include "matchmaking.h"

DECLARE_string(app_flavor);

namespace {

const WallClock::Duration kOneSecond = WallClock::FromMsec(1000);

class PongServer : public Component {
 public:
  static bool Install(const ArgumentMap &arguments) {
    LOG(INFO) << "Built using Engine version: " << FUNAPI_BUILD_IDENTIFIER;

    if (FLAGS_app_flavor == "lobby") {
      LOG(INFO) << "Install lobby server";
    } else if (FLAGS_app_flavor == "login") {
      LOG(INFO) << "Install login server";
    } else if (FLAGS_app_flavor == "matchmaker") {
      LOG(INFO) << "Install matchmaker  server";
      pong::StartMatchmakingServer();
    } else {
      LOG(INFO) << "Install game server";
    }

    // 초기화 부분입니다.
    pong::ObjectModelInit();
    pong::RegisterEventHandlers();
    return true;
  }

  static bool Start() {
    return true;
  }

  static bool Uninstall() {
    return true;
  }

};

}  // unnamed namespace

REGISTER_STARTABLE_COMPONENT(PongServer, PongServer)
