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

}  // namespace pong

#endif  // SRC_PONG_TYPES_H_
