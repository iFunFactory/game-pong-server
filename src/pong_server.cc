#include <boost/bind.hpp>
#include <funapi.h>
#include <gflags/gflags.h>

#include "event_handlers.h"
#include "pong_object.h"

DECLARE_string(app_flavor);

namespace {

const WallClock::Duration kOneSecond = WallClock::FromMsec(1000);

class PongServer : public Component {

public:
static bool Install(const ArgumentMap &arguments) {
  LOG(INFO) << "Built using Engine version: " << FUNAPI_BUILD_IDENTIFIER;

  if (FLAGS_app_flavor == "lobby") {
    LOG(INFO) << "Install lobby server";
  } else if (FLAGS_app_flavor == "matchmaker") {
    LOG(INFO) << "Install matchmaker server";
  } else if (FLAGS_app_flavor == "login") {
    LOG(INFO) << "Install login server";
  } else {
    LOG(INFO) << "Install game server";
  }

  // 초기화 부분입니다.
  pong::ObjectModelInit();
  pong::RegisterEventHandlers();
  // MatchmakingServer를 시작합니다.
  MatchmakingServer::Start(CheckMatch, CheckCompletion, OnJoined, OnLeft);
  return true;
}


// player 가 match 에 참여해도 되는지 검사합니다.
static bool CheckMatch(const MatchmakingServer::Player &player, const MatchmakingServer::Match &match) {
  // 조건없이 바로 매칭합니다.
  return true;
}


// JoinMatch 함수가 불린 후 호출됩니다. 해당 매치가 성사 되었는지 판단합니다.
static MatchmakingServer::MatchState CheckCompletion(const MatchmakingServer::Match &match) {
  LOG(INFO) << "[" << FLAGS_app_flavor  << "] CheckCompletion " << match.players.size();
  if (match.players.size() == 2)
    return MatchmakingServer::kMatchComplete;
  else
    return MatchmakingServer::kMatchNeedMorePlayer;
}


// CheckMatch 함수에서 조건에 만족하여 true 가 반환되면 이 함수가 호출됩니다. 이제 플레이어는 match 에 참여하게 되었습니다.
static void OnJoined(const MatchmakingServer::Player &player, MatchmakingServer::Match *match) {
  LOG(INFO) << "[" << FLAGS_app_flavor <<"] OnJoined " + player.id;
  if (match->context.IsNull()) {
    match->context.SetObject();
    match->context["A"] = player.id;
  } else {
    match->context["B"] = player.id;
  }
}


static void OnLeft(const MatchmakingServer::Player &player, MatchmakingServer::Match *match) {
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
