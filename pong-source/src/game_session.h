#ifndef SRC_GAME_SESSION_H_
#define SRC_GAME_SESSION_H_

#include <funapi.h>

#include <array>
#include <memory>
#include <string>
#include <vector>


namespace pong {

class GameSession : public boost::enable_shared_from_this<GameSession> {
 public:
  static Ptr<GameSession> Create(
      const EncodingScheme scheme,
      int32_t index,
      const fun::Uuid &game_id,
      const std::vector<std::string> &users);

  virtual ~GameSession();

  bool Join(const Ptr<Session> &session);
  void SetReady(const Ptr<Session> &session);
  void SetResult(const Ptr<Session> &session);
  void RelayMessage(const Ptr<Session> &session, const fun::Json &msg);
  void RelayMessage(const Ptr<Session> &session, const Ptr<FunMessage> &msg);

  // TCP 연결이 끊긴 경우
  void SetDetached(const Ptr<Session> &session);

  // 방에서 나간 경우
  // True를 반환하는 경우, 방을 종료한다.
  bool Leave(const std::string &uid);

 protected:
  GameSession(EncodingScheme scheme,
              int32_t index,
              const fun::Uuid &game_id);

  void CheckStartTimeout();
  void DestroySession();

 public:
  const int32_t game_index_;
  const fun::Uuid id_;

 private:
  struct User {
   std::string uid_;
   Ptr<Session> session_;
   bool ready_;
  };

  std::array<User, 2> users_;
  const EncodingScheme scheme_;
};

}  // namespace pong

#endif  // SRC_GAME_SESSION_H_