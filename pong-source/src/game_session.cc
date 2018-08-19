#include "game_session.h"

#include <gflags/gflags.h>

#include "common_handlers.h"
#include "game_event_handlers.h"
#include "leaderboard.h"


using std::vector;
using std::string;


DEFINE_string(pong_game_result_url, "", "API Endpoint for game results");


namespace pong {

namespace {

// Player 들의 승/패를 1 증가 시킵니다.
void FetchAndUpdateMatchRecord(const string& winner_id,
                               const string& loser_id) {
#if PONG_ENABLE_ORM
  Ptr<User> winner = User::FetchById(winner_id);
  Ptr<User> loser = User::FetchById(loser_id);

  if(not winner) {
    LOG(ERROR) << "Cannot find winner's id in db: id=" << winner_id;
    return;
  }

  if(not loser) {
    LOG(ERROR) << "Cannot find loser's id in db: id=" << loser_id;
    return;
  }

  winner->SetWinCount(winner->GetWinCount() + 1);
  loser->SetLoseCount(loser->GetLoseCount() + 1);
#endif
}


void SendResultMessage(const Ptr<Session> &session,
                       bool win,
                       EncodingScheme scheme) {
  if (not session) {
    return;
  }

  if (scheme == kJsonEncoding) {
    session->SendMessage("result", MakeResponse(win ? "win" : "lose"));
  } else {
    Ptr<FunMessage> msg = boost::make_shared<FunMessage>();
    msg->MutableExtension(game_result)->set_result(win ? "win" : "lose");
    session->SendMessage("result", msg);
  }
}

}  // unnamed namespace


Ptr<GameSession> GameSession::Create(EncodingScheme scheme,
                                     int32_t index,
                                     const fun::Uuid &id,
                                     const vector<string>& users) {
  auto game = Ptr<GameSession>(new GameSession(scheme, index, id));
  BOOST_ASSERT(users.size() == 2);
  for (size_t i = 0; i < users.size(); ++i) {
    game->users_[i].uid_ = users[i];
    game->users_[i].ready_ = false;
  }

  game->CheckStartTimeout();

  return game;
}


GameSession::GameSession(
    EncodingScheme scheme, int32_t index, const fun::Uuid &id)
  : game_index_(index), id_(id), scheme_(scheme) {
  DLOG(INFO) << "GameSession(index=" << index
             << ", id=" << id << "): created";
}


GameSession::~GameSession() {
  DLOG(INFO) << "GameSession(index=" << game_index_
             << ", id=" << id_ << "): destroyed";
}


bool GameSession::Join(const Ptr<Session> &session) {
  // 항상 game_id 에 해당하는 이벤트 태그로 실행해야 함
  BOOST_ASSERT(GetCurrentEventTag() == id_);

  const std::string uid = AccountManager::FindLocalAccount(session);
  for (auto &user : users_) {
    if (user.uid_ == uid) {
      LOG_IF(WARNING, user.session_)
          << "Game(" << id_
          << "): User(" << uid << "): already has a session";
      user.session_ = session;
      return true;
    }
  }
  return false;
}


void GameSession::SetReady(const Ptr<Session> &session) {
  // 항상 game_id 에 해당하는 이벤트 태그로 실행해야 함
  BOOST_ASSERT(GetCurrentEventTag() == id_);

  bool ready = true;
  for (auto &user : users_) {
    if (user.session_ == session) {
      user.ready_ = true;
    } else {
      ready = user.ready_ && ready;
    }
  }

  if (not ready) {
    return;
  }

  // 모두 준비 완료
  if (scheme_ == kJsonEncoding) {
    Json response = MakeResponse("ok");
    for (auto &user : users_) {
      user.session_->SendMessage("start", response);
    }
  } else {
    Ptr<FunMessage> msg(new FunMessage);
    GameStartMessage *start_msg = msg->MutableExtension(game_start);
    start_msg->set_result("ok");
    for (auto &user : users_) {
      user.session_->SendMessage("start", msg);
    }
  }
}


void GameSession::RelayMessage(
    const Ptr<Session> &session, const fun::Json &msg) {
  // 항상 game_id 에 해당하는 이벤트 태그로 실행해야 함
  BOOST_ASSERT(GetCurrentEventTag() == id_);
  BOOST_ASSERT(scheme_ == kJsonEncoding);

  // NOTE(jinuk): 필요에 따라서 TCP, UDP 등을 고를 수도 있다.
  for (auto &user : users_) {
    if (user.session_ && user.session_ != session) {
      user.session_->SendMessage("relay", msg);
      return;
    }
  }
}


void GameSession::RelayMessage(
    const Ptr<Session> &session, const Ptr<FunMessage> &msg) {
  // 항상 game_id 에 해당하는 이벤트 태그로 실행해야 함
  BOOST_ASSERT(GetCurrentEventTag() == id_);
  BOOST_ASSERT(scheme_ == kProtobufEncoding);

  // NOTE(jinuk): 필요에 따라서 TCP, UDP 등을 고를 수도 있다.
  for (auto &user : users_) {
    if (user.session_ && user.session_ != session) {
      user.session_->SendMessage("relay", msg);
      return;
    }
  }
}


void GameSession::SetResult(const Ptr<Session> &session) {
  BOOST_ASSERT(GetCurrentEventTag() == id_);

  // 이 메시지는 패배한 쪽에서 전달한다.
  // FIXME(jinuk): 서버에서 relay 메시지를 분석해서 승패 확인
  std::string winner, loser;

  for (auto &user : users_) {
    if (user.session_ == session) {
      // 패배 메시지 전달
      loser = user.uid_;
      SendResultMessage(user.session_, false, scheme_);
    } else {
      // 승리 메시지 전달
      winner = user.uid_;
      SendResultMessage(user.session_, true, scheme_);
    }
  }

  if (not winner.empty()) {
    IncreaseCurWinCount(winner);
  }
  if (not loser.empty()) {
    ResetCurWinCount(loser);
  }

  fun::Event::Invoke([game=shared_from_this(), winner, loser](){
      FetchAndUpdateMatchRecord(winner, loser);
      game->DestroySession();
    }, id_);

  // 매치 결과를 싱글 플레이 서버로 전송
  /*
   * 여기서 HttpClient 를 만들고,
   * header 에 application/json 설정
   * body 에 game_id / 승리한 유저 아이디 / 패배한 유저 아이디 기록
   * 해당 header+body 를 싱글 플레이 웹 서버 주소에 전송
   */
  if (FLAGS_pong_game_result_url.empty()) {
    LOG(ERROR) << "pong_game_result_url is not set in MANIFEST.json";
    return;
  }
  Ptr<HttpClient> client = boost::make_shared<HttpClient>();
  client->SetHeader("Content-Type", "applicatoin/json");
  fun::Json result;
  result.SetObject();
  result.AddAttribute("game_id", boost::lexical_cast<string>(id_));
  result.AddAttribute("winner", winner);
  result.AddAttribute("loser", loser);
  auto callback = [client](const CURLcode code, const http::Response &res) {
      if (code != CURLE_OK) {
        LOG(ERROR) << "Failed to post match result to "
                   << FLAGS_pong_game_result_url
                   << "; curl=" << static_cast<int>(code);
        return;
      }

      if (res.status_code != fun::http::kOk) {
        LOG(ERROR) << "Failed to post match result to "
                   << FLAGS_pong_game_result_url
                   << "; HTTP status=" << static_cast<int>(res.status_code);
        return;
      }
    };
  client->PostAsync(FLAGS_pong_game_result_url, result, callback, 30000);
}


void GameSession::SetDetached(const Ptr<Session> &session) {
  // NOTE(jinuk): TCP 연결이 끊겼을 때 처리할 게 있다면 여기서 처리한다.
}


bool GameSession::Leave(const std::string &uid) {
  BOOST_ASSERT(GetCurrentEventTag() == id_);
  bool empty = true;

  for (auto &user : users_) {
    if (user.uid_ == uid) {
      user.session_.reset();
    } else {
      // 이미 남은 유저가 있다면 플래그 수정
      if (user.session_) {
        empty = false;
      }
    }
  }

  if (not empty) {
    DestroySession();  // 나머지 유저를 로비로 돌려보낸다.
    return false;
  }

  return empty;
}


void GameSession::CheckStartTimeout() {
  // NOTE(jinuk): 유저가 방을 만들고 상대방이 30초 동안 안들어오면 방폭파
  fun::Timer::ExpireAfter(fun::WallClock::FromSec(30),
      [game=this->shared_from_this()](
          const fun::Timer::Id&, const fun::WallClock::Value&) {
        for (const auto &user : game->users_) {
          if (not user.session_ || not user.ready_) {
            game->DestroySession();
            return;
          }
        }
      }, id_);
}


void GameSession::DestroySession() {
  for (auto &u : users_) {
    if (u.session_) {
      MoveServerByTag(u.session_, "lobby", fun::Json());
      u.session_.reset();
    }
  }

  fun::Event::Invoke([id=id_]() {
    TerminateGame(id);
  }, id_);
}

}  // namespace pong