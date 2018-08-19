#ifndef SRC_LEADERBOARD_H_
#define SRC_LEADERBOARD_H_

#include <funapi.h>


namespace pong {

int GetCurrentRecordById(const string &id, bool single = false);
void IncreaseCurWinCount(const string &id, bool single = false);
void ResetCurWinCount(const string &id, bool single = false);
void GetAndSendTopEightList(
    const Ptr<Session> session, EncodingScheme encoding, bool single = false);


using TopEightListCallback = boost::function<
    void (const LeaderboardQueryRequest &request,
          const LeaderboardQueryResponse &response,
          const bool &error)>;

void GetTopEightList(bool single, const TopEightListCallback &callback);

}  // namespace pong

#endif  // SRC_LEADERBOARD_H_

