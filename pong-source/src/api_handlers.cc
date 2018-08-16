#include <funapi.h>
#include <funapi/types.h>

#include <gflags/gflags.h>
#include <sodium/utils.h>
#include <sodium/randombytes.h>

#include "leaderboard.h"


namespace pong {

namespace {

const auto kContentType = std::make_pair("Content-Type", "application/json");


void SetErrorResponse(const Ptr<fun::http::Response> &res,
                      const fun::http::StatusCode code,
                      int64_t error_code,
                      const std::string &msg) {
  res->header.insert(kContentType);
  res->status_code = code;
  fun::Json body;
  body.SetObject();
  body.AddAttribute("error_code", error_code);
  if (not msg.empty()) {
    body.AddAttribute("msg", msg);
  }
  res->body = body.ToString(false);
}


void SetResponse(const Ptr<http::Response> &res,
                 const fun::http::StatusCode code,
                 const fun::Json &result) {
  res->header.insert(kContentType);
  res->status_code = code;
  res->body = result.ToString();
}


static void OnUserConnectionRequest(
    const http::Request2 &request, const ApiService::MatchResult &,
    const ApiService::ResponseWriter &writer) {
  // 1. HTTP 요청을 확인
  // HTTP request body 에 JSON 형식으로 다음 유저 데이터가 있다고 가정
  fun::Json body;
  if (not body.FromString(request.body) ||
      not body.IsObject() ||
      not body.HasAttribute("user", fun::Json::kObject)) {
    LOG(ERROR) << "Request is not valid: body=" << request.body;
    auto response = boost::make_shared<http::Response>();
    SetErrorResponse(response, http::kBadRequest, 400, "Invalid request body");
    writer(response);
    return;
  }

  fun::Json user_data = body["user"];
  // 적어도 uid 필드는 있어야 한다. (문자열로 가정)
  if (not user_data.HasAttribute("uid", fun::Json::kString)) {
    LOG(ERROR) << "Missging uid field: user_data="
               << user_data.ToString(false);
    auto response = boost::make_shared<http::Response>();
    SetErrorResponse(response, http::kBadRequest, 400, "Invalid uid field");
    writer(response);
    return;
  }

  // 2. 적당한 매치메이킹 서버를 찾는다.
  fun::Rpc::PeerMap peers;
  fun::Rpc::PeerId peer_id;
  if (0 < fun::Rpc::GetPeersWithTag(&peers, "lobby", true)) {
    // NOTE: 여기서는 단순히 랜덤하게 고른다.
    auto it = peers.begin();
    std::advance(it, fun::RandomGenerator::GenerateNumber(0, peers.size() - 1));
    peer_id = it->first;
  } else {
    // 매치메이킹 서버가 한 대도 없는 경우
    LOG(ERROR) << "No matchmaking lobby servers.";
    auto response = boost::make_shared<http::Response>();
    SetErrorResponse(response,
                     http::kInternalServerError,
                     500,
                     "Not enough matchmaking servers");
    writer(response);
    return;
  }

  const std::string host = fun::Rpc::GetPeerExternalHostname(peer_id);
  // NOTE: 아이펀 엔진은 여러 개의 포트를 열 수 있다. 여기선 단순히 TCP포트만
  //       쓰는 것처럼 가정한다. (TCP + protobuf)
  auto ports = fun::Rpc::GetPeerExternalPorts(peer_id);
  auto pi = ports.find(fun::HardwareInfo::kTcpPbuf);
  if (pi == ports.end()) {
    // TCP가 없는 경우 (설정 잘못한 경우)
    LOG(ERROR) << "No open TCP port.";
    auto response = boost::make_shared<http::Response>();
    SetErrorResponse(response,
                     http::kInternalServerError,
                     500,
                     "No open TCp port");
    writer(response);
    return;
  }

  uint16_t port = pi->second;

  // 3. 인증 토큰을 생성
  // TODO: 원하는 형식의 인증 토큰을 생성. 여기에선 단순 랜덤 토큰을 생성함.
  std::array<uint8_t, 16> token_buf;
  randombytes_buf(&token_buf[0], token_buf.size());
  std::string token;
  token.resize(token_buf.size() * 2 + 1);
  sodium_bin2hex(&token[0], token.size(), &token_buf[0], token_buf.size());
  token.resize(token.size() - 1);

  // 4. Redis 에 토큰 저장. 저장에 성공하면 반환.
  //    토큰은 최대 300초 동안 유지한다. (그 이후에는 토큰 없어져서 인증 실패)
  fun::Redis::SetExAsync(token, 300, body["user"].ToString(false),
      [writer, host, port, token](const fun::Redis::Result &result) {
        if (result != fun::Redis::kResultSuccess) {
          LOG(ERROR) << "Failed to set key in redis.";
          auto response = boost::make_shared<http::Response>();
          SetErrorResponse(response,
                           http::kInternalServerError,
                           500,
                           "Failed to update redis");
          writer(response);
        } else {
          fun::Json res;
          res.SetObject();
          res.AddAttribute("error_code", 0);
          res.AddAttribute("token", token);
          res.AddAttribute("host", host);
          res.AddAttribute("port", port);

          auto response = boost::make_shared<http::Response>();
          SetResponse(response, http::kOk, res);
          writer(response);
        }
      });
}


static void OnRankingRequest(
    const http::Request2 &request, const ApiService::MatchResult &param,
    const ApiService::ResponseWriter &writer) {
  const bool is_single = param[0] == "single";

  GetTopEightList(is_single,
      [writer](const LeaderboardQueryRequest &rank_request,
               const LeaderboardQueryResponse &rank_response,
               const bool &error) {
        fun::Json res;
        res.SetObject();
        if (error) {
          res.AddAttribute("error_code", 1000);
          auto response = boost::make_shared<http::Response>();
          SetResponse(response, http::kOk, res);
          writer(response);
          return;
        }

        res.AddAttribute("error_code", 0);
        fun::Json ranks;
        for (const auto &record : rank_response.records) {
          Json rank;
          rank.SetObject();
          rank.AddAttribute("rank", record.rank);
          rank.AddAttribute("score", record.score);
          rank.AddAttribute("id", record.player_account.id());
          ranks.PushBack(rank);
        }

        ranks.Move(&res["ranks"]);
        auto response = boost::make_shared<http::Response>();
        SetResponse(response, http::kOk, res);
        writer(response);
      });
}

}  // of anonymous namespace


void RegisterAPIHandlers() {
  // 싱글 플레이 서버에서 유저 이동 시키는 경우
  ApiService::RegisterHandler3(
      http::kPost, boost::regex("/v1/user-connection-request/"),
      OnUserConnectionRequest);

  // leaderboard
  ApiService::RegisterHandler3(
      http::kGet, boost::regex("/v1/ranking/(?<type>(single|multi))/"),
      OnRankingRequest);
}

}  // of namespace pong
