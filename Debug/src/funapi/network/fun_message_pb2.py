# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: funapi/network/fun_message.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)




DESCRIPTOR = _descriptor.FileDescriptor(
  name='funapi/network/fun_message.proto',
  package='',
  serialized_pb='\n funapi/network/fun_message.proto\"^\n\nFunMessage\x12\x0b\n\x03sid\x18\x01 \x01(\t\x12\x0f\n\x07msgtype\x18\x02 \x01(\t\x12\x0b\n\x03seq\x18\x03 \x01(\r\x12\x0b\n\x03\x61\x63k\x18\x04 \x01(\r\x12\x0e\n\x06urgent\x18\x05 \x01(\x08*\x08\x08\x08\x10\x80\x80\x80\x80\x02')




_FUNMESSAGE = _descriptor.Descriptor(
  name='FunMessage',
  full_name='FunMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='sid', full_name='FunMessage.sid', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='msgtype', full_name='FunMessage.msgtype', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='seq', full_name='FunMessage.seq', index=2,
      number=3, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='ack', full_name='FunMessage.ack', index=3,
      number=4, type=13, cpp_type=3, label=1,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='urgent', full_name='FunMessage.urgent', index=4,
      number=5, type=8, cpp_type=7, label=1,
      has_default_value=False, default_value=False,
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
  is_extendable=True,
  extension_ranges=[(8, 536870912), ],
  serialized_start=36,
  serialized_end=130,
)

DESCRIPTOR.message_types_by_name['FunMessage'] = _FUNMESSAGE

class FunMessage(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _FUNMESSAGE

  # @@protoc_insertion_point(class_scope:FunMessage)


# @@protoc_insertion_point(module_scope)
