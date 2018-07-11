#include "leaderboard.h"

#include <funapi.h>
#include <glog/logging.h>

#include "pong_messages.pb.h"


DECLARE_string(app_flavor);


// 현재 연승 기록
static const char *kPlayerCurWinCount = "player_cur_wincount";
// 1 일 최대 연승 기록
static const char *kPlayerRecordWinCount = "player_record_wincount";

static const char *kServiceProvider = "test_kServiceProvider";


namespace pong {

// 리더보드에 기록된 최대 연승을 가져옵니다.
int GetCurrentRecordById(const string &id) {
  // 랭킹 조회 요청을 만듭니다.
  LeaderboardQueryRequest request(
      kPlayerCurWinCount, kServiceProvider, id, kDaily,
      LeaderboardRange(LeaderboardRange::kNearby, 0, 0));

  // 랭킹을 조회합니다.
  // TODO: 비동기로 변경
  LeaderboardQueryResponse response;
  if (not GetLeaderboardSync(request, &response)) {
    LOG(ERROR) << "leaderboard system error";
    return 0;
  }

  // 현재 연승 수를 반환합니다.
  if (response.records.empty()) {
    return 0;
  }
  return response.records[0].score;
}


// 1일 최대 연승 기록을 업데이트 후 불립니다.
void OnNewRecordSubmitted(
  const string &id, const ScoreSubmissionRequest &request,
  const ScoreSubmissionResponse &response, const bool &error) {
  if (error) {
    LOG(ERROR) << "Failed to update score. Leaderboard system error: id=" << id;
    return;
  }

  switch (response.result) {
    case kNewRecord: {
      LOG(INFO) << "New record: id=" << id << ", score=" << response.new_score;
      break;
    }
    case kNewRecordMonthly: {
      LOG(INFO) << "New monthly record: id=" << id << ", score="
                << response.new_score;
      break;
    }
    case kNewRecordWeekly: {
      LOG(INFO) << "New weekly record: id=" << id << ", score="
                << response.new_score;
      break;
    }
    case kNewRecordDaily: {
      LOG(INFO) << "New daily record: id=" << id << ", score="
                << response.new_score;
      break;
    }
    case kNone: {
      break;
    }
    default: {
       BOOST_ASSERT(false);
    }
  }
}


// 1 일 최대 연승 기록을 업데이트합니다.
void UpdateNewRecord(const string &id, const int score) {
  ScoreSubmissionRequest request(
      kPlayerRecordWinCount, kServiceProvider, id, score,
      ScoreSubmissionRequest::kHighScore);
  SubmitScore(request, bind(&OnNewRecordSubmitted, id, _1, _2, _3));
}


// 현재 연승 기록을 1 증가 시킨 후 불립니다.
void OnIncreaseCurWinCountSubmitted(
    const string &id, const ScoreSubmissionRequest &request,
    const ScoreSubmissionResponse &response, const bool &error) {
  if (error) {
    LOG(ERROR) << "Failed to update score. Leaderboard system error: id=" << id;
    return;
  }

  switch (response.result) {
    case kNewRecord:
    case kNewRecordMonthly:
    case kNewRecordWeekly:
    case kNewRecordDaily:
    case kNone: {
      // 1 씩 증가하기 떄문에 항상 new record 가 됩니다.
      break;
    } default: {
      LOG(ERROR) << "Ignoring invalid leaderboard response: "
                 << response.result;
      return;
    }
  }

  // 1 일 최대 연승 기록을 업데이트합니다.
  // TODO: 1 일 최대 연승 값을 ORM 에 기록하고 갱신이 확실할 때만 호출한다.
  UpdateNewRecord(request.player_account.id(), response.new_score);
}


// 현재 연승 기록을 1 증가 시킵니다.
void IncreaseCurWinCount(const string &id) {
  ScoreSubmissionRequest request(
      kPlayerCurWinCount, kServiceProvider, id, 1,
      ScoreSubmissionRequest::kIncrement);
  SubmitScore(request, bind(&OnIncreaseCurWinCountSubmitted, id, _1, _2, _3));
}


// 현재 연승 기록이 0 으로 초기화된 후 불립니다.
void OnResetCurWinCountSubmitted(
    const string &id, const ScoreSubmissionRequest &request,
    const ScoreSubmissionResponse &response,
    const bool &error) {
  if (error) {
    LOG(ERROR) << "Failed to reset score. Leaderboard system error: id=" << id;
    return;
  }
}


// 현재 연승 기록을 0 으로 초기화 합니다.
void ResetCurWinCount(const string &id) {
  ScoreSubmissionRequest request(
      kPlayerCurWinCount, kServiceProvider, id, 0,
      ScoreSubmissionRequest::kOverwriting);
  SubmitScore(request, bind(&OnResetCurWinCountSubmitted, id, _1, _2, _3));
}


// 1 일 최대 연승 기록 TOP 8 을 가져온 후 불립니다.
void OnGetTopEightList(
    const Ptr<Session> session, const LeaderboardQueryRequest &request,
    const LeaderboardQueryResponse &response, const bool &error,
    EncodingScheme encoding) {
  if (error) {
    LOG(ERROR) << "Failed to query top 8. Leaderboard system error.";
    return;
  }

  if (encoding == kJsonEncoding) {
    Json msg;
    for (int i = 0; i < response.total_player_count; ++i) {
      string index = std::to_string(i);
      msg[index]["rank"] = response.records[i].rank;
      msg[index]["score"] = response.records[i].score;
      msg[index]["id"] = response.records[i].player_account.id();
    }

    session->SendMessage("ranklist", msg, kDefaultEncryption);
  } else {
    Ptr<FunMessage> msg(new FunMessage);
    LobbyRankListReply *rank_response
        = msg->MutableExtension(lobby_rank_list_repl);
    rank_response->set_result("Success");
    for (int i = 0; i < response.total_player_count; ++i) {
      LobbyRankListReply::RankElement *elem = rank_response->add_rank();
      elem->set_rank(response.records[i].rank);
      elem->set_score(response.records[i].score);
      elem->set_id(response.records[i].player_account.id());
    }
    session->SendMessage("ranklist", msg, kDefaultEncryption);
  }
}


// 1 일 최대 연승 기록 TOP 8 을 세션으로 전송합니다.
void GetAndSendTopEightList(const Ptr<Session> session,
                            EncodingScheme encoding) {
  LeaderboardQueryRequest request(
    kPlayerRecordWinCount, kDaily,
    LeaderboardRange(LeaderboardRange::kFromTop, 0, 7),
    LeaderboardQueryRequest::kStdCompetition);
  GetLeaderboard(
      request, bind(&OnGetTopEightList, session, _1, _2, _3, encoding));
}

} // namespace pong
