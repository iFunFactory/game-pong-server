#ifndef SRC_COMMON_HANDLERS_H_
#define SRC_COMMON_HANDLERS_H_

#include <funapi.h>

#include "pong_messages.pb.h"
#include "pong_object.h"
#include "pong_rpc_messages.pb.h"
#include "pong_types.h"

namespace pong {

void MoveServerByTag(const Ptr<Session> session, const string &tag);
void RegisterCommonHandlers();

}  // namespace pong

#endif  // SRC_COMMON_HANDLERS_H_

