# vim: fileencoding=utf-8 ts=4 sts=4 sw=4 et
import os
from flask import Flask

app = Flask(__name__)
app.config.from_object('pong_web.settings')


if os.environ.has_key('PONG_WEB_SETTINGS'):
    os.app.config.from_envvar('PONG_WEB_SETTINGS')


from pong_web import views
