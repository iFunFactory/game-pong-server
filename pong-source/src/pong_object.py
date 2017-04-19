#!/usr/bin/env python
# vim: fileencoding=utf-8 tabstop=2 softtabstop=2 shiftwidth=2 expandtab
# Copyright (C) 2013-2016 iFunFactory Inc. All Rights Reserved.
#
# This work is confidential and proprietary to iFunFactory Inc. and
# must not be used, disclosed, copied, or distributed without the prior
# consent of iFunFactory Inc.


# Example.
#
# AppId/ProjectName
#   MyGame
#
# Object Model
#
#   {
#     "Character": {
#       "Name": "String KEY",
#       "Level": "Integer",
#       "Hp": "Integer",
#       "Mp": "Integer"
#     }
#   }
#
# Create/Fetch/Delete Example
#
#   import my_game_object
#
#   def Example():
#     # Initialize
#     # my_game_object.initialize(mysql id, mysql pw, mysql server address, database name, zookeeper hosts, app id)
#     my_game_object.initialize('db_id', 'db_pw', '127.0.0.1', 'my_game_db', '127.0.0.1', 'MyGame')
#
#     # Create an object
#     character = my_game_object.Character.create('mycharacter')
#     character.set_Level(1)
#     character.set_Hp(150)
#     character.set_Mp(70)
#     character.commit()
#
#     # Fetch an object
#     character = my_game_object.Character.fetch_by_Name('mycharacter2')
#     character.set_Level(character.get_Level() + 1)
#     print character.get_Name()
#     print character.get_Level()
#     print character.get_Hp()
#     print character.get_Mp()
#     character.commit()
#
#     # Delete an object
#     character = my_game_object.Character.fetch_by_Name('mycharacter3')
#     character.delete()
#     character.commit()


import binascii
import sys
sys.path.append('/usr/share/funapi/python')

import funapi.object.object as funapi


def initialize(user, password, host, database, zookeeper_hosts, app_name):
  funapi.MysqlConnection.initialize(user, password, host, database)
  funapi.Zookeeper.initialize(zookeeper_hosts, app_name)


object = funapi.ObjectModel('User')
attribute = funapi.AttributeModel('Id', 'String', True, False, False, False, False, '')
object.add_attribute_model(attribute)
attribute = funapi.AttributeModel('WinCount', 'Integer', False, False, False, False, False, '')
object.add_attribute_model(attribute)
attribute = funapi.AttributeModel('LoseCount', 'Integer', False, False, False, False, False, '')
object.add_attribute_model(attribute)
attribute = funapi.AttributeModel('_tag', 'String', False, False, False, False, False, '')
object.add_attribute_model(attribute)
funapi.ObjectModel.add_object_model(object)
del attribute
del object


class User:
  @staticmethod
  def create(Id):
    model = funapi.ObjectModel.get_object_model('User')
    attributes = {}
    attributes['Id'] = Id
    obj = funapi.Object.create(model, attributes)
    return User(obj)

  @staticmethod
  def fetch(object_id):
    if len(object_id) == 16:
      object_id = binascii.hexlify(object_id)
    model = funapi.ObjectModel.get_object_model('User')
    obj = funapi.Object.fetch(model, object_id)
    if obj == None:
      return None
    return User(obj)

  @staticmethod
  def fetch_by_Id(value):
    model = funapi.ObjectModel.get_object_model('User')
    obj = funapi.Object.fetch_by(model, 'Id', value)
    if obj == None:
      return None
    return User(obj)

  def __init__(self, obj):
    self.object_ = obj

  def commit(self):
    self.object_.commit()

  def delete(self):
    self.object_.delete()

  def get_object_id(self):
    return self.object_.get_object_id()

  def get_Id(self):
    return self.object_.get_attribute('Id')

  def set_Id(self, value):
    self.object_.set_attribute('Id', value)

  def get_WinCount(self):
    return self.object_.get_attribute('WinCount')

  def set_WinCount(self, value):
    self.object_.set_attribute('WinCount', value)

  def get_LoseCount(self):
    return self.object_.get_attribute('LoseCount')

  def set_LoseCount(self, value):
    self.object_.set_attribute('LoseCount', value)

  def get__tag(self):
    return self.object_.get_attribute('_tag')

  def set__tag(self, value):
    self.object_.set_attribute('_tag', value)

