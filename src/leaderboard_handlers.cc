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
			// system error
			LOG(ERROR) << "leaderboard system error";
			return 0;
		}

		return response.records[0].score;
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

	void OnCurWincountSubmitted(
			const ScoreSubmissionRequest &request,
			const ScoreSubmissionResponse &response,
			const bool &error) {
		if (error) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] leaderboard system error";
			return;
		}

		LOG(INFO) << "[" << FLAGS_app_flavor << "] current score: " << response.new_score;

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

	void UpdateCurWincount(const string &id) {
		ScoreSubmissionRequest request(
				kPlayerCurWincount,
				service_provider,
				id,
				1,
				ScoreSubmissionRequest::kIncrement);

		ScoreSubmissionResponseHandler handler = OnCurWincountSubmitted;
		SubmitScore(request, handler);
	}

	void OnWincountToZeroSubmitted(
			const ScoreSubmissionRequest &request,
			const ScoreSubmissionResponse &response,
			const bool &error) {
		if (error) {
			LOG(ERROR) << "[" << FLAGS_app_flavor << "] leaderboard system error";
			return;
		}

//		 BOOST_ASSERT(false);
	}

	void SetWincountToZero(const string &id) {
		ScoreSubmissionRequest request(
				kPlayerCurWincount,
				service_provider,
				id,
				0,
				ScoreSubmissionRequest::kOverwriting);

		LOG(INFO) << id;

		ScoreSubmissionResponseHandler handler = OnWincountToZeroSubmitted;
		SubmitScore(request, handler);
	}

	void OnResponse(
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


 	void GetListTopEight(const Ptr<Session> session) {
		LeaderboardQueryRequest request(
			kPlayerRecordWincount,
			kDaily,
			LeaderboardRange(LeaderboardRange::kFromTop, 0, 7),
			LeaderboardQueryRequest::kStdCompetition);

		LeaderboardQueryResponseHandler handler = bind(&OnResponse, session, _1, _2, _3);
		GetLeaderboard(request, handler);
	}
} // namespace pong_lb
