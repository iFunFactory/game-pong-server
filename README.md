# Pong Game Server

이 프로젝트는 iFun Engine을 사용하는 Unity3d 사용자를 위한 샘플 게임 서버입니다. 해당 게임 서버를 테스트하기 위한 환경은 다음과 같습니다.

* 권장 운영체제 Ubuntu 16.04
* mysql 혹은 mariadb
* zookeeper
* funapi-leaderboard

**해당 문서에서는 Ubuntu 16.04에서 설정하는 방법을 서술합니다.**

## 목차

* [다운로드](#다운로드)
* [서버 환경 설정](#서버-환경-설정)
    - [mysql 설치](#mysql-설치)
    - [mysql 설치 후 환경설정](#mysql-설치-후-환경설정)
    - [zookeeper 설치](#zookeeper-설치)
    - [funapi-leaderboard 설치](#funapi-leaderboard-설치)
    - [funapi-leaderboard 설치 후 환경설정](#funapi-leaderboard-설치-후-환경설정)
* [프로젝트 디렉터리 구조](#프로젝트-디렉터리-구조)
* [빌드 환경 구축](#빌드-환경-구축)
* [게임 서버 빌드](#게임-서버-빌드)
* [게임 서버 실행](#게임-서버-실행)
* [테스트](#테스트)

## 다운로드

**Pong Server** 프로젝트를 **git clone** 으로 다운 받거나 **zip 파일** 을 다운 받아 주세요.

```bash
git clone https://github.com/iFunFactory/game-pong-server.git
```

## 서버 환경 설정

Pong-server를 구동하기 위해 `mysql`, `zookeeper`, `redis-server`, `funapi-leaderboard`의 설치, 환경설정이 필요합니다. 각 개발환경 설정에 대한 자세한 내용은 [튜토리얼](https://www.ifunfactory.com/engine/documents/tutorial/ko/project.html#object-relational-mapping-db)과 [메뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/development-environment.html)을 참고해 주세요.

#### mysql 설치
`mysql server`가 설치되어있지 않다면 아래의 명령어를 통해 mysql server를 설치해주세요.

```bash
$ sudo apt-get install mysql-server
```

#### mysql 설치 후 환경설정

```bash
$ sudo service mysql start

$ mysql -u root -p

mysql> create user 'funapi'@'localhost' identified by 'funapi';

mysql> grant all privileges on *.* to 'funapi'@'localhost';

mysql> create database funapi;

mysql> create database funapi_leaderboard;
```

#### zookeeper 설치

```bash
$ sudo apt-get install zookeeper zookeeperd
$ sudo service zookeeper start
```

#### redis-server 설치

```bash
$ sudo apt-get install redis-server
$ sudo service redis-server start
```

#### funapi-leaderboard 설치

leaderboard는 agent구조로 되어있습니다. 아래의 명령어를 이용해 설치합니다.

```bash
$ sudo apt-get update
$ sudo apt-get install funapi-leaderboard1
```

또한 리더보드는 캐시 처리를 위해 `Redis` 를 사용합니다. 아래 명령어를 이용하여 Redis를 설치합니다.

```bash
$ sudo apt-get install redis-server
```

#### funapi-leaderboard 설치 후 환경설정

리더보드 에이전트가 정상적으로 설치되었다면 `/usr/share/funapi-leaderboard/default/manifests/MANIFEST.json` 설정파일이 생성됩니다.

`MANIFEST.json` 파일을 열고, 아래와 같이 변경해주세요.

```bash
$ sudo vim /usr/share/funapi-leaderboard/default/manifests/MANIFEST.json
```

```json
{
  ...
  "components": [
    {
      ...
      "dependency": {
        ...
        "Redis": {
          "enable_redis": true,
          "redis_mode": "redis",
          "redis_servers": {
            "": {
              "address": "127.0.0.1:6379",
              "auth_pass": ""
            }
          },
          "redis_async_threads_size": 10
        }
      },
      "arguments": {
        "server_tcp_port": 12820,
        "mysql_server_url": "tcp://127.0.0.1:3306",
        "mysql_id": "funapi",
        "mysql_pw": "funapi",
        "mysql_db_name": "funapi_leaderboard",
        "mysql_db_connection_count": 10,
        "reset_schedules": [
          {
            "leaderboard_id": "player_cur_wincount",
            "period": "day",
            "interval": 1,
            "starts": "2015-01-01 05:00:00",
            "ends": "2020-12-31 23:00:00"
          },
          {
            "leaderboard_id": "player_cur_wincount",
            "period": "week",
            "interval": 1,
            "starts": "2015-01-01 05:00:00",
            "ends": "2020-12-31 23:00:00"
          },
          {
            "leaderboard_id": "player_record_wincount",
            "period": "day",
            "interval": 1,
            "starts": "2015-01-01 05:00:00",
            "ends": "2020-12-31 23:00:00"
          },
          {
            "leaderboard_id": "player_record_wincount",
            "period": "week",
            "interval": 1,
            "starts": "2015-01-01 05:00:00",
            "ends": "2020-12-31 23:00:00"
          }
        ]
      },
      "library": "libfunapi-leaderboard.so"
    }
  ]
}
```

* `leaderboard_id` : 랭킹을 구분하기 위한 이름으로 player_cur_wincount는 플레이어의 현재 연승 수를, player_record_wincount는 플레이어의 최고 연승 수를 의미합니다.
* `period` : 랭킹 리셋 주기를 의미합니다.
* `interval` : 랭킹을 리셋하는 주기의 간격을 입력합니다.
* `start` : 랭킹을 리셋하는 스케쥴의 최초 시작 날짜와 시간을 입력합니다.
* `ends` : 랭킹을 리셋하는 스케쥴의 종료 날짜와 시간을 입력합니다.

설정이 완료되면 다음 명령어를 통해 실행합니다.

```bash
$ sudo service funapi-leaderboard start
```

`MANIFEST.json` 의 각 Argument들에 대한 자세한 정보는 관련 [메뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/leaderboard-service.html#leaderboard-service-agent-configuration)을 참고해주세요.

## 프로젝트 디렉터리 구조
다운로드와 서버 환경 설정이 완료되었다면 `game-pong-server` 디렉터리로 가봅시다. source 디렉터리에는 여러 파일들이 있는데, 각각을 간략히 설명하자면 다음과 같습니다. 자세한 내용은 [메뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/development-environment.html#source)을 참고해 주세요.

* `VERSION`: 생성되는 게임의 버전 숫자를 기록합니다.
* `DEBIAN`: 생성되는 게임을 DEB 패키지 형태로 묶는 경우 DEB 패키지 생성에 필요한 내용들을 기술합니다.
* `LICENSE`: 배포시에 적용할 사용권 등을 설명하실 수 있습니다.
* `README`: 내부적으로 게임 소스에 대한 설명 글을 남겨야될 때 이 파일을 이용하시면 됩니다.
* `setup_build_environment`: source 디렉터리를 이용해서 빌드 디렉터리를 생성할 때 실행될 스크립트입니다.
* `CMakeLists.txt`: 프로젝트의 최상위 레벨 cmake 입력 파일입니다.

* **`src/`**: 여러분이 직접 추가/삭제할 게임 코드들은 대개는 이 디렉토리 안에 들어가게 됩니다.

* `src/CMakeLists.txt`: cmake 입력 파일이며, 게임 개발자들이 소스 파일을 추가하거나 삭제할 때 실질적으로 쓰일 파일입니다.
* `src/pong_types.h`: 아이펀 엔진 게임에서 사용되는 타입에 대한 정의를 담고 있는 파일입니다.
* `src/pong_loggers.json`: 해당 게임 프로젝트의 activity logger 정의 파일입니다.
* `src/pong_server.cc`: 해당 게임의 main 에 해당하는 함수입니다.
* `src/event_handlers.h`: 핸들러의 선언을 기록합니다. 필요에 따라 파일 이름을 바꾸거나 나눌 수 있습니다.
* `src/event_handlers.cc`: src/event_handlers.h 에 정의된 핸들러들의 구현이 포함되어있습니다.
* `src/pong_utils.h`: 유틸 함수들의 선언을 기록합니다.
* `src/pong_utils.cc`: src/pong_utils.h에 정의된 함수들의 구현이 포함되어있습니다.
* `src/rpc_handlers.h`: rpc와 관련된 핸들러들의 선언을 기록합니다.
* `src/rpc_handlers.cc`: src/rpc_handlers.h에 정의된 핸들러들의 구현이 포함되어있습니다.
* `src/leaderboard_handlers.h`: 리더보드와 관련된 핸들러들의 선언을 기록합니다.
* `src/leaderboard_handlers.cc`: src/leaderboard_handlers.h에 정의된 핸들러들의 구현이 포함되어있습니다.
* `src/MANIFEST.*.json`: src/pong_server.cc 에 정의된 game server component 를 포함해서 서버별로 각 component 들에 대한 의존성과 component 의 설정이 포함됩니다.
* **`src/object_model/`**: Pong object를 정의하는 file 들이 위치하는 directory 입니다.
* `src/object_model/pong.json`: Pong object를 정의하는 file 입니다.

## 빌드 환경 구축

이제 빌드 환경을 구축해 보겠습니다. game-pong-server를 다운로드 시 `git clone` 을 수행하였거나 압축을 해제한 디렉터리. 즉, `game-pong-server` 디렉터리가 보이는 디렉터리에서 다음의 명령어를 입력합니다.

```bash
game-pong-server/setup_build_environment --type=makefile
```

명령어를 실행시키면 빌드 환경 생성이 진행됨을 알리는 로그가 출력됩니다. 빌드 환경 생성이 완료되면 프로젝트 최상위 디렉터리에 `pong-build` 폴더가 새롭게 생성됩니다. `pong-build` 디렉터리 하위에는 `debug`와 `release` 디렉터리가 생성되는데, 각각의 디렉터리에서 debug버전 빌드와 release버전의 빌드를 할 수 있습니다.

## 게임 서버 빌드
`pong-build/debug` 디렉터리로 이동하여 `make`명령을 입력해 봅시다. 다음과 같은 메시지가 출력되었다면 성공입니다.

```bash
$ cd pong-build/debug
$ make

...
Linking CXX shard module libpong.so
[100%] Built target Pong
```

iFunEngine은 빌드 후 `-local` 스크립트와 `-launcher` 스크립트를 생성합니다. `-local` 스크립트는 개발 중에 서버를 실행할 때 사용하며 `-launcher` 스크립트는 게임 서버를 패키징하여 데몬으로 실행할 때 사용합니다. 또한, 해당 프로젝트에서는 flavor 기능을 사용하여 lobby, matchmaker, game으로 나누어 서버를 관리하고 있습니다. 때문에 빌드가 완료되면 lobby, matchmaker, game서버마다 별도로 `-local`스크립트와 `-launcher`스크립트가 생성됩니다.

* `lobby server` : 로그인 처리를 하며 matchmaking 대기 중인 클라이언트가 머무르는 서버입니다.
* `matchmaker server` : matchmaking 을 처리하는 서버입니다.
* `game server` : 매칭된 클라이언트가 머무르는 서버입니다.

flavor에 대한 자세한 내용은 [메뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/game-management.html#flavors)을 참고해 주세요.

## 게임 서버 실행

서버를 실행하기 전에, **분산 서비스** 설정을 위해 RPC 서비스 설정이 필요합니다. 각 서버의 설정파일은 `manifest` 디렉터리 하위에 있는 lobby, matchmaker, game 디렉터리 안에 생성됩니다.
먼저, `ifconfig`명령 혹은 `ip link` 명령으로 네트워크 인터페이스 이름을 확인해주세요.

```bash
$ ifconfig
# 또는
$ ip link
```

네트워크 인터페이스 이름을 확인하셨으면, lobby 서버의 `MANIFEST.json` 파일을 열어주세요.

```
$ sudo vim manifest/lobby/MANIFEST.json
```

아래의 `rpc_nic_name` 내용을 `ifconfig`에서 확인한 네트워크 인터페이스 이름으로 변경해주세요.

```json
...
"RpcService": {
            "rpc_enabled": true,
            "rpc_threads_size": 4,
            "rpc_port": 6015,
            "rpc_nic_name": "eth0",
            "rpc_tags": [],
            "rpc_message_logging_level": 0,
            "enable_rpc_reply_checker": true
          },
...
```

완료하셨으면, game, matchmaker 서버의 `MANIFEST.json` 파일도 동일하게 수정해주세요.

```bash
$ sudo vim manifest/game/MANIFEST.json
$ sudo vim manifest/matchmaker/MANIFEST.json
```

이제 테스트를 위해서, 각각의 `-local` 스크립트들을 실행시켜주세요.

```bash
$ ./pong.lobby-local
$ ./pong.matchmaker-local
$ ./pong.game-local
```

아래와 같이 출력되면 게임 서버가 성공적으로 실행된 것입니다!

```text
...
I0109 00:00:00.094714  9203 manifest_handler.cc:742] Starting SessionService
I0109 00:00:00.094841  9203 session_service.cc:1001] tcp json server start: port(8012)
I0109 00:00:00.094918  9203 session_service.cc:1015] udp json server start: port(8013)
I0109 00:00:00.094981  9203 manifest_handler.cc:742] Starting PongServer
```

## 테스트

먼저, [여기](https://github.com/iFunFactory/game-pong)에서 pong 게임 클라이언트를 다운받아주세요.

다음으로, 다운받은 Pong Client 프로젝트를 실행시켜 Main씬을 로드합니다. `NetworkManager` 오브젝트 의 Server addr값을 현재 서버의 주소로 변경해주세요.

게임을 실행시키고 **[게스트 로그인]** 혹은 **[페이스북 로그인]** 버튼을 누르면 **클라이언트**에서 **lobby 서버**로 로그인 요청을 보내게 됩니다. **lobby 서버**가 요청을 받아 정상적으로 인증되면 클라이언트를 **lobby server**로 이동시키고 클라이언트에서는 **[대전시작]** 버튼이 활성화됩니다. **lobby 서버**에서는 아래와 같은 로그인 성공 메시지를 출력함을 확인할 수 있습니다.

```bash
...
I0412 16:25:51.575713 29140 transport.cc:292] Client plugin version: 186
I0412 16:25:51.577785 29140 session_service.cc:1262] session created: 3645f3c2-820f-42dc-a6e4-1083cbe8aebc
I0412 16:25:51.601130 29206 lobby_event_handlers.cc:141] Succeed to login: id=2976b445e50b4257b94d51086f437ec25f358671_Desktop
```

**[대전시작]** 버튼을 누르면 **클라이언트**에서 **lobby 서버**로 매치 요청을 보내게 되고, **lobby 서버**는 **matchmaker 서버**에게 매치메이킹을 요청합니다. 매치가 정상적으로 이루어지면 두 **클라이언트**는 **game 서버**로 이동하고 대전이 진행됩니다. 이후 승패가 결정되면 두 클라이언트는 다시 **lobby 서버**로 돌아오게 됩니다.

일정 시간이 지나도 상대를 찾지 못할 경우 Timeout되어 매칭을 취소하게 됩니다.

```bash
...
I0109 00:00:00.632324  9555 event_handlers.cc:271] Failed in matchmaking. Timeout: id=4f4ccf9233f6cd83978a5bd21ad41e1e61829d81_Editor
```

**[순위]** 버튼을 누르면 daily 랭킹을 확인할 수 있습니다. 순위는 매일 05시에 갱신됩니다.
