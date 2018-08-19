import datetime

#from sqlalchemy.dialects import mysql
from sqlalchemy import func

from pong_web import db


class User(db.Model):
  __tablename__ = 'pong_user'

  user_id = db.Column(db.Integer, primary_key=True, autoincrement=True)
  name = db.Column(db.String(32), unique=True)
  win_count = db.Column(db.Integer)
  lose_count = db.Column(db.Integer)
  single_win_count = db.Column(db.Integer)
  single_lose_count = db.Column(db.Integer)

  def __init__(self, user_id):
    self.user_id = user_id
    self.name = 'user' + str(user_id)
    self.win_count = 0
    self.lose_count = 0
    self.single_win_count = 0
    self.single_lose_count = 0

  def to_dict(self):
    return {
      'id': self.user_id,
      'uid': str(self.user_id),
      'name': self.name,
      'win_count': self.win_count,
      'lose_count': self.lose_count,
      'single_win_count': self.single_win_count,
      'single_lose_count': self.single_lose_count,
    }
