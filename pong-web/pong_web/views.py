# vim: fileencoding=utf-8 tabstop=4 softtabstop=4 shiftwidth=4 expandtab

from __future__ import print_function

from functools import wraps
import json
import os
import sys

import flask
from flask import abort, g, jsonify, request, url_for, redirect

from werkzeug.contrib.securecookie import SecureCookie
import requests


from pong_web import app


try:
    import pkg_resources
    __version__ = pkg_resources.get_distribution('pong_web').version
except:
    __version__ = 'dev'


_TOKEN_KEY = 'X-Ife-Auth'


class JsonSecureCookie(SecureCookie):
    serialize_method = json


def token_required(fn):
    @wraps(fn)
    def wrapped(*args, **kwargs):
        if _TOKEN_KEY not in request.headers:
            abort(400, 'Token required')

        # TODO(jinuk): token 유효성 검사
        token = SecureCookie.unserialize(
                request.headers[_TOKEN_KEY], app.config['SECRET_KEY'])
        g.token = token
        return fn(*args, **kwargs)

    return wrapped


@app.context_processor
def version():
    return dict(version=__version__, app_name=app.config['APP_NAME'])


@app.route('/')
def index():
    return 'OK'


@app.route('/v1/login', methods=['POST'])
def handle_login():
    try:
        body = json.loads(request.data)
    except:
        import traceback
        print(traceback.format_exc())
        abort(400, 'Invalid JSON')

    if 'name' not in body or 'password' not in body:
        return jsonify(error_code=400, msg='Missing required field(s)')

    if not body['name'].startswith('user'):
        return jsonify(error_code=401, msg='Invalid username or password')

    try:
        uid = body['name'][4:]
    except:
        print('Invalid uid:', body['name'])
        return jsonify(error_code=401, msg='Invalid username or password')

    if body['password'] == 'blah':
        user_data = {
            'uid': uid,
        }
        token = SecureCookie(
                user_data, secret_key=app.config['SECRET_KEY']).serialize()
        return jsonify(error_code=0, token=token)
    else:
        return jsonify(error_code=401, msg='Invalid username or password')


@app.route('/v1/multi-play', methods=['POST'])
@token_required
def handle_multi_play():
    try:
        body = json.loads(request.data)
    except:
        import traceback
        print(traceback.format_exc())
        abort(400)

    try:
        req = requests.post('http://localhost:6014/v1/user-connection-request/',
                data=json.dumps({'user': {'uid': g.token['uid']}}))
        req.raise_for_status()
        result = json.loads(req.content)
    except:
        import traceback
        print(traceback.format_exc())

    return json.dumps(result)


def get_ranks(single):
    url = 'http://localhost:6014/v1/ranking/'
    if single:
        url += 'single/'
    else:
        url += 'multi/'

    try:
        req = requests.get(url)
        req.raise_for_status()
        result = json.loads(req.content)
        return json.dumps(result)
    except:
        import traceback
        print(traceback.format_exc())
        return jsonify(error_code=1001)


@app.route('/v1/ranking/single/', methods=['GET'])
@token_required
def handle_single_ranking():
    return get_ranks(True)


@app.route('/v1/ranking/multi/', methods=['GET'])
@token_required
def handle_multi_ranking():
    return get_ranks(False)


@app.route('/v1/match/single', strict_slashes=False, methods=['POST'])
@token_required
def handle_single_match_finished():
    return jsonify(error_code=0)


@app.route('/v1/match/result', strict_slashes=False, methods=['POST'])
def handle_match_result():
    # TODO(jinuk): 여기서 매치 결과를 처리한다.
    return jsonify(error_code=0)

