#ifndef SRC_RPC_HANDLERS_H_
#define SRC_RPC_HANDLERS_H_

#include <funapi.h>

#include "pong_messages.pb.h"
#include "pong_object.h"
#include "pong_rpc_messages.pb.h"
#include "pong_types.h"

namespace pong_rpc {

void MatchmakingRpc(const Ptr<Session> session);
void CancelMatchmakingRpc(const Ptr<Session> session);
void CancelMatchmakingRpcByTcpDetached(const Ptr<Session> session);
void RegisterRpcHandlers();

}  // namespace pong_rpc

#endif  // SRC_RPC_HANDLERS_H_

