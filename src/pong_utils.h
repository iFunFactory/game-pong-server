#ifndef SRC_PONG_UTILS_H_
#define SRC_PONG_UTILS_H_

#include <funapi.h>

#include "pong_messages.pb.h"
#include "pong_object.h"
#include "pong_rpc_messages.pb.h"
#include "pong_types.h"

namespace pong_util {

        void MoveServerByTag(const Ptr<Session> session, const string &tag);

	void RegisterRedirectionHandlers();

}  // namespace pong_util

#endif  // SRC_PONG_UTILS_H_

