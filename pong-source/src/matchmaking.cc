#include <funapi.h>

#include "matchmaking.h"
#include "pong_types.h"


namespace pong {

namespace {

// player 가 match 에 참여해도 되는지 검사합니다.
bool CheckJoinable(const MatchmakingServer::Player &player,
                   const MatchmakingServer::Match &match) {
  BOOST_ASSERT(match.type == kMatch1vs1);

  // 조건없이 바로 매칭합니다.
  return true;
}


// JoinMatch 함수가 불린 후 호출됩니다. 해당 매치가 성사 되었는지 판단합니다.
static MatchmakingServer::MatchState CheckCompletion(
    const MatchmakingServer::Match &match) {
  BOOST_ASSERT(match.type == kMatch1vs1);

  if (match.players.size() == 2) {
    LOG(INFO) << "Match completed: team_a=" << match.context["A"].GetString()
              << ", team_b=" << match.context["B"].GetString();
    return MatchmakingServer::kMatchComplete;
  }
  return MatchmakingServer::kMatchNeedMorePlayer;
}


// CheckMatch 함수에서 조건에 만족하여 true 가 반환되면 이 함수가 호출됩니다.
// 이제 플레이어는 match 에 참여하게 되었습니다.
void OnJoined(const MatchmakingServer::Player &player,
              MatchmakingServer::Match *match) {
  BOOST_ASSERT(match->type == kMatch1vs1);

  // 팀을 구성합니다. 1vs1 만 있기 때문에 각 플레이어를 A, B 팀으로 나눕니다.
  if (not HasJsonStringAttribute(match->context, "A")) {
    match->context["A"] = player.id;
  } else {
    BOOST_ASSERT(not HasJsonStringAttribute(match->context, "B"));
    match->context["B"] = player.id;
  }
}


void OnLeft(const MatchmakingServer::Player &player,
            MatchmakingServer::Match *match) {
  BOOST_ASSERT(match->type == kMatch1vs1);

  // OnJoined 와 반대로 팀에서 해당 플레이어를 삭제합니다.
  // 1vs1 이기 때문에 팀을 삭제합니다.
  if (HasJsonStringAttribute(match->context, "A") &&
      match->context["A"].GetString() == player.id) {
    match->context.RemoveAttribute("A");
  } else {
    BOOST_ASSERT(HasJsonStringAttribute(match->context, "B") &&
                 match->context["B"].GetString() == player.id);
    match->context.RemoveAttribute("B");
  }
}

}  // unnamed namespace


void StartMatchmakingServer() {
  MatchmakingServer::Start(CheckJoinable, CheckCompletion, OnJoined, OnLeft);
}

}  // namespace pong
