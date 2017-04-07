#ifndef SRC_MATCHMAKING_H_
#define SRC_MATCHMAKING_H_

#include <funapi.h>

#include "pong_types.h"

namespace pong {

enum MatchmakingType {
  kMatch1vs1 = 0
};


void StartMatchmakingServer();

}  // namespace pong


#endif  // SRC_MATCHMAKING_H_

