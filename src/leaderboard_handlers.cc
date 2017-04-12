#include "leaderboard_handlers.h"

#include <funapi.h>
#include <glog/logging.h>

#include "pong_loggers.h"
#include "pong_messages.pb.h"

DECLARE_string(app_flavor);

namespace pong_lb {
	const char *kPlayerCurWincount = "player_cur_wincount";
	const char *kPlayerRecordWincount = "player_record_wincount";
	const char *service_provider = "test_service_provider";

	int GetCurrentRecordById(const string &id) {
		LeaderboardQueryRequest request(
				kPlayerCurWincount,
				service_provider,
				id,
				kDaily,
				LeaderboardRange(LeaderboardRange::kNearby, 0, 0));

		// 랭킹을 조회합니다.
		LeaderboardQueryResponse response;
		if (not GetLeaderboardSync(request, &response)) {
			LOG(ERROR) << "leaderboard system error";
			return 0;
		}

		// 현재 연승 수를 반환합니다.
		return response.records[0].score;
	}

	void OnNewRecordSubmitted(
		const ScoreSubmissionRequest &request,
		const ScoreSubmissionResponse &response,
		const bool &error) {

		if (error) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] leaderboard system error";
			return;
		}

		LOG(INFO) << "[" << FLAGS_app_flavor << "] new record: " << response.new_score;

		switch (response.result) {
			case kNewRecord: {
				break;
			}
			case kNewRecordWeekly: {
				break;
			}
			case kNewRecordDaily: {
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

	void UpdateNewRecord(const string &id, const int score) {
		ScoreSubmissionRequest request(
				kPlayerRecordWincount,
				service_provider,
				id,
				score,
				ScoreSubmissionRequest::kHighScore);

		ScoreSubmissionResponseHandler handler = OnNewRecordSubmitted;
		SubmitScore(request, handler);
	}

	void OnIncreaseCurWincountSubmitted(
			const ScoreSubmissionRequest &request,
			const ScoreSubmissionResponse &response,
			const bool &error) {
		if (error) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] leaderboard system error";
			return;
		}

		LOG(INFO) << "[" << FLAGS_app_flavor << "] id : " << request.player_account.id() << " current score: " << response.new_score;

		switch (response.result) {
			case kNewRecord: {
				break;
			}
			case kNewRecordWeekly: {
				break;
			}
			case kNewRecordDaily: {
				break;
			}
			case kNone: {
				break;
			}
			default: {
				 BOOST_ASSERT(false);
			}
		}

		UpdateNewRecord(request.player_account.id(), response.new_score);
	}

	void IncreaseCurWincount(const string &id) {
		ScoreSubmissionRequest request(
				kPlayerCurWincount,
				service_provider,
				id,
				1,
				ScoreSubmissionRequest::kIncrement);

		ScoreSubmissionResponseHandler handler = OnIncreaseCurWincountSubmitted;
		SubmitScore(request, handler);
	}

	void OnResetCurWincountSubmitted(
			const ScoreSubmissionRequest &request,
			const ScoreSubmissionResponse &response,
			const bool &error) {
		if (error) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] leaderboard system error";
			return;
		}
	}

	void ResetCurWincount(const string &id) {
		ScoreSubmissionRequest request(
				kPlayerCurWincount,
				service_provider,
				id,
				0,
				ScoreSubmissionRequest::kOverwriting);


		ScoreSubmissionResponseHandler handler = OnResetCurWincountSubmitted;
		SubmitScore(request, handler);
	}

	void OnGetTopEightList(
			const Ptr<Session> session,
			const LeaderboardQueryRequest &request,
			const LeaderboardQueryResponse &response,
			const bool &error) {

		if (error) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] leaderboard system error";
			return;
		}

		Json msg;

		for (int i = 0; i < response.total_player_count; ++i) {
			string index = std::to_string(i);

			msg[index]["rank"] = response.records[i].rank;
			msg[index]["score"] = response.records[i].score;
			msg[index]["id"] = response.records[i].player_account.id();
		}

		session->SendMessage("ranklist", msg, kDefaultEncryption, kTcp);
	}


	void GetTopEightList(const Ptr<Session> session) {
		LeaderboardQueryRequest request(
			kPlayerRecordWincount,
			kDaily,
			LeaderboardRange(LeaderboardRange::kFromTop, 0, 7),
			LeaderboardQueryRequest::kStdCompetition);

		LeaderboardQueryResponseHandler handler = bind(&OnGetTopEightList, session, _1, _2, _3);
		GetLeaderboard(request, handler);
	}
} // namespace pong_lb
