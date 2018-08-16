from pong_web import app


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=9000, debug=True, use_reloader=True)
