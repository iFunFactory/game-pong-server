#include <boost/bind.hpp>
#include <funapi.h>
#include <gflags/gflags.h>

#include "game_event_handlers.h"
#include "lobby_event_handlers.h"
#include "matchmaking.h"
#include "pong_object.h"


DECLARE_string(app_flavor);

namespace {

const WallClock::Duration kOneSecond = WallClock::FromMsec(1000);

class PongServer : public Component {
 public:
  static bool Install(const ArgumentMap &arguments) {
    LOG(INFO) << "Built using Engine version: " << FUNAPI_BUILD_IDENTIFIER;

    pong::ObjectModelInit();

    if (FLAGS_app_flavor == "lobby") {
      // Lobby 서버 역할로 초기화 합니다.
      LOG(INFO) << "Install lobby server";
      pong::RegisterLobbyEventHandlers();
    } else if (FLAGS_app_flavor == "game") {
      // Game 서버 역할로 초기화 합니다.
      LOG(INFO) << "Install game server";
      pong::RegisterGameEventHandlers();
    } else if (FLAGS_app_flavor == "matchmaker") {
      // Matchmaker 서버 역할로 초기화 합니다.
      LOG(INFO) << "Install matchmaker  server";
      pong::StartMatchmakingServer();
    } else {
      BOOST_ASSERT(false);
    }

    return true;
  }

  static bool Start() {
    LOG(INFO) << "Starting " << FLAGS_app_flavor << " server";
    return true;
  }

  static bool Uninstall() {
    return true;
  }

};

}  // unnamed namespace

REGISTER_STARTABLE_COMPONENT(PongServer, PongServer)
