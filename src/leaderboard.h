#ifndef SRC_LEADERBOARD_H_
#define SRC_LEADERBOARD_H_

#include <funapi.h>


namespace pong {

int GetCurrentRecordById(const string &id);
void IncreaseCurWinCount(const string &id);
void ResetCurWinCount(const string &id);
void GetAndSendTopEightList(const Ptr<Session> session);

}  // namespace pong

#endif  // SRC_LEADERBOARD_H_

