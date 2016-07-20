# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: pong_messages.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)


import funapi.network.fun_message_pb2
import funapi.service.multicast_message_pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='pong_messages.proto',
  package='',
  serialized_pb='\n\x13pong_messages.proto\x1a funapi/network/fun_message.proto\x1a&funapi/service/multicast_message.proto\"\x1e\n\x0fPbufEchoMessage\x12\x0b\n\x03msg\x18\x01 \x02(\t\"!\n\x12PbufAnotherMessage\x12\x0b\n\x03msg\x18\x01 \x01(\t\"#\n\x10PbufHelloMessage\x12\x0f\n\x07message\x18\x01 \x01(\t:0\n\tpbuf_echo\x12\x0b.FunMessage\x18\x10 \x01(\x0b\x32\x10.PbufEchoMessage:6\n\x0cpbuf_another\x12\x0b.FunMessage\x18\x11 \x01(\x0b\x32\x13.PbufAnotherMessage:;\n\npbuf_hello\x12\x14.FunMulticastMessage\x18\t \x01(\x0b\x32\x11.PbufHelloMessage')


PBUF_ECHO_FIELD_NUMBER = 16
pbuf_echo = _descriptor.FieldDescriptor(
  name='pbuf_echo', full_name='pbuf_echo', index=0,
  number=16, type=11, cpp_type=10, label=1,
  has_default_value=False, default_value=None,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)
PBUF_ANOTHER_FIELD_NUMBER = 17
pbuf_another = _descriptor.FieldDescriptor(
  name='pbuf_another', full_name='pbuf_another', index=1,
  number=17, type=11, cpp_type=10, label=1,
  has_default_value=False, default_value=None,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)
PBUF_HELLO_FIELD_NUMBER = 9
pbuf_hello = _descriptor.FieldDescriptor(
  name='pbuf_hello', full_name='pbuf_hello', index=2,
  number=9, type=11, cpp_type=10, label=1,
  has_default_value=False, default_value=None,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)


_PBUFECHOMESSAGE = _descriptor.Descriptor(
  name='PbufEchoMessage',
  full_name='PbufEchoMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='msg', full_name='PbufEchoMessage.msg', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=97,
  serialized_end=127,
)


_PBUFANOTHERMESSAGE = _descriptor.Descriptor(
  name='PbufAnotherMessage',
  full_name='PbufAnotherMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='msg', full_name='PbufAnotherMessage.msg', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=129,
  serialized_end=162,
)


_PBUFHELLOMESSAGE = _descriptor.Descriptor(
  name='PbufHelloMessage',
  full_name='PbufHelloMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='message', full_name='PbufHelloMessage.message', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
  ],
  options=None,
  is_extendable=False,
  extension_ranges=[],
  serialized_start=164,
  serialized_end=199,
)

DESCRIPTOR.message_types_by_name['PbufEchoMessage'] = _PBUFECHOMESSAGE
DESCRIPTOR.message_types_by_name['PbufAnotherMessage'] = _PBUFANOTHERMESSAGE
DESCRIPTOR.message_types_by_name['PbufHelloMessage'] = _PBUFHELLOMESSAGE

class PbufEchoMessage(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _PBUFECHOMESSAGE

  # @@protoc_insertion_point(class_scope:PbufEchoMessage)

class PbufAnotherMessage(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _PBUFANOTHERMESSAGE

  # @@protoc_insertion_point(class_scope:PbufAnotherMessage)

class PbufHelloMessage(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _PBUFHELLOMESSAGE

  # @@protoc_insertion_point(class_scope:PbufHelloMessage)

pbuf_echo.message_type = _PBUFECHOMESSAGE
funapi.network.fun_message_pb2.FunMessage.RegisterExtension(pbuf_echo)
pbuf_another.message_type = _PBUFANOTHERMESSAGE
funapi.network.fun_message_pb2.FunMessage.RegisterExtension(pbuf_another)
pbuf_hello.message_type = _PBUFHELLOMESSAGE
funapi.service.multicast_message_pb2.FunMulticastMessage.RegisterExtension(pbuf_hello)

# @@protoc_insertion_point(module_scope)
