FILE(REMOVE_RECURSE
  "CMakeFiles/unity_dll"
  "../unity_dll/messages.dll"
  "../unity_dll/FunMessageSerializer.dll"
  "../unity_dll/protobuf-net.dll"
  "funapi/distribution/fun_rpc_message.pb.cc"
  "funapi/distribution/fun_rpc_message.pb.h"
  "funapi/distribution/fun_rpc_message_pb2.py"
  "funapi/service/multicast_message.pb.cc"
  "funapi/service/multicast_message.pb.h"
  "funapi/service/multicast_message_pb2.py"
  "funapi/management/maintenance_message.pb.cc"
  "funapi/management/maintenance_message.pb.h"
  "funapi/management/maintenance_message_pb2.py"
  "funapi/network/ping_message.pb.cc"
  "funapi/network/ping_message.pb.h"
  "funapi/network/ping_message_pb2.py"
  "funapi/network/fun_message.pb.cc"
  "funapi/network/fun_message.pb.h"
  "funapi/network/fun_message_pb2.py"
  "pong_messages.pb.cc"
  "pong_messages.pb.h"
  "pong_messages_pb2.py"
  "pong_rpc_messages.pb.cc"
  "pong_rpc_messages.pb.h"
  "pong_rpc_messages_pb2.py"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/unity_dll.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
