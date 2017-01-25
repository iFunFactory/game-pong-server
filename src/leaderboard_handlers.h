#ifndef SRC_LEADERBOARD_HANDLERS_H_
#define SRC_LEADERBOARD_HANDLERS_H_

#include <funapi.h>

#include "pong_messages.pb.h"
#include "pong_object.h"
#include "pong_rpc_messages.pb.h"
#include "pong_types.h"

namespace pong_lb {

int GetCurrentRecordById(const string &id);
void UpdateCurWincount(const string &id);
void SetWincountToZero(const string &id);
void GetListTopEight(const Ptr<Session> session);

}  // namespace pong_lb

#endif  // SRC_LEADERBOARD_HANDLERS_H_

