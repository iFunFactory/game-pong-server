#ifndef SRC_PONG_TYPES_H_
#define SRC_PONG_TYPES_H_

#include <funapi.h>


namespace pong {

using fun::string;
using fun::shared_ptr;
using fun::weak_ptr;
using fun::Uuid;


// JSON Object 에 지정된 String Attribute 가 있는지 검사합니다.
inline bool HasJsonStringAttribute(const Json &json,
                                   const string &attribute_name) {
  if (not json.IsObject()) {
    return false;
  }
  if (not json.HasAttribute(attribute_name)) {
    return false;
  }
  if (not json[attribute_name].IsString()) {
    return false;
  }
  return true;
}


inline Json MakeResponse(const string &result, const string &message) {
  Json response;
  response["result"] = result;
  response["message"] = message;
  return response;
}


inline Json MakeResponse(const string &result) {
  Json response;
  response["result"] = result;
  return response;
}


inline Rpc::PeerId PickServerRandomly(const Rpc::Tag &tag) {
  Rpc::PeerMap servers;
  Rpc::GetPeersWithTag(&servers, tag);
  if (servers.empty()) {
    return Rpc::kNullPeerId;
  }
  int64_t rnd = RandomGenerator::GenerateNumber(0, servers.size() - 1);
  Rpc::PeerMap::const_iterator itr = servers.begin();
  for (int64_t i = 0; i < rnd; ++i) { ++itr; }
  return itr->first;
}

}  // namespace pong

#endif  // SRC_PONG_TYPES_H_
