# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: funapi/management/maintenance_message.proto

from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)


import funapi.network.fun_message_pb2


DESCRIPTOR = _descriptor.FileDescriptor(
  name='funapi/management/maintenance_message.proto',
  package='',
  serialized_pb='\n+funapi/management/maintenance_message.proto\x1a funapi/network/fun_message.proto\"L\n\x12MaintenanceMessage\x12\x12\n\ndate_start\x18\x01 \x01(\t\x12\x10\n\x08\x64\x61te_end\x18\x02 \x01(\t\x12\x10\n\x08messages\x18\x03 \x01(\t::\n\x10pbuf_maintenance\x12\x0b.FunMessage\x18\x0f \x01(\x0b\x32\x13.MaintenanceMessage')


PBUF_MAINTENANCE_FIELD_NUMBER = 15
pbuf_maintenance = _descriptor.FieldDescriptor(
  name='pbuf_maintenance', full_name='pbuf_maintenance', index=0,
  number=15, type=11, cpp_type=10, label=1,
  has_default_value=False, default_value=None,
  message_type=None, enum_type=None, containing_type=None,
  is_extension=True, extension_scope=None,
  options=None)


_MAINTENANCEMESSAGE = _descriptor.Descriptor(
  name='MaintenanceMessage',
  full_name='MaintenanceMessage',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='date_start', full_name='MaintenanceMessage.date_start', index=0,
      number=1, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='date_end', full_name='MaintenanceMessage.date_end', index=1,
      number=2, type=9, cpp_type=9, label=1,
      has_default_value=False, default_value=unicode("", "utf-8"),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='messages', full_name='MaintenanceMessage.messages', index=2,
      number=3, type=9, cpp_type=9, label=1,
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
  serialized_start=81,
  serialized_end=157,
)

DESCRIPTOR.message_types_by_name['MaintenanceMessage'] = _MAINTENANCEMESSAGE

class MaintenanceMessage(_message.Message):
  __metaclass__ = _reflection.GeneratedProtocolMessageType
  DESCRIPTOR = _MAINTENANCEMESSAGE

  # @@protoc_insertion_point(class_scope:MaintenanceMessage)

pbuf_maintenance.message_type = _MAINTENANCEMESSAGE
funapi.network.fun_message_pb2.FunMessage.RegisterExtension(pbuf_maintenance)

# @@protoc_insertion_point(module_scope)
