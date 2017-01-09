# Pong Game Server

이 프로젝트는 iFun Engine을 사용하는 Unity3d 사용자를 위한 샘플 게임 서버입니다. 해당 게임 서버를 테스트하기 위한 환경은 다음과 같습니다.

* Ubuntu 14.04 혹은 16.04, CentOS 6 혹은 7
* mysql 혹은 mariadb
* zookeeper

**해당 문서에서는 Ubuntu 14.04에서 설정하는 방법을 서술합니다.**

## 목차

* [다운로드](#다운로드)
* [서버 환경 설정](#서버-환경-설정)
* [프로젝트 구조](#프로젝트-구조)
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

Pong-server를 구동하기 위해 `mysql`과 `zookeeper`의 설치, 환경설정이 필요합니다. 개발환경 설정에 대한 자세한 내용은 [튜토리얼](https://www.ifunfactory.com/engine/documents/tutorial/ko/project.html#object-relational-mapping-db)과 [메뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/development-environment.html)을 참고해 주세요.

#### mysql 설치
`mysql server`가 설치되어있지 않다면 다음처럼 하시면 됩니다.

```bash
$ sudo apt-get install mysql-server
```

#### mysql 설치 후 환경설정

```bash
$ mysql -u root -p

mysql> create user 'funapi'@'localhost' identified by 'funapi';

mysql> grant all privileges on *.* to 'funapi'@'localhost';

mysql> create database funapi;

$ sudo service mysql start
```

#### zookeeper 설치

```bash
$ sudo apt-get install zookeeper zookeeperd
$ sudo service zookeeper start
```


## 프로젝트 구조
다운로드가 완료되었다면 `game-pong-server` 디렉터리로 가 봅시다. source 디렉터리에는 여러 파일들이 있는데, 각각을 간략히 설명하자면 다음과 같습니다. 자세한 내용은 [메뉴얼](https://www.ifunfactory.com/engine/documents/reference/ko/development-environment.html#source)을 참고해 주세요.

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
* `src/event_handlers.h`: pong.cc 는 게임이 처리할 event 와 client message 들을 위한 핸들러들을 등록합니다.
* `src/event_handlers.cc`: src/event_handelrs.h 에 정의된 핸들러들의 구현이 포함되어있습니다.
* `src/MANIFEST.json`: pong.cc 에 정의된 game server component 를 포함해서 각 component 들에 대한 의존성과 component 의 설정이 포함됩니다.
* **`src/object_model/`**: Pong object를 정의하는 file 들이 위치하는 directory 입니다.
* `src/object_model/pong.json`: Pong object를 정의하는 file 입니다.

## 빌드 환경 구축

이제 빌드 환경을 구축해 보겠습니다. game-pong-server를 다운로드 시 `git clone` 을 수행 혹은 압축을 해제한 `game-pong-server` 디렉터리가 보이는 디렉터리에서 다음의 명령어를 입력합니다.

```bash
game-pong-server/setup_build_environment --type==makefile
```

명령어를 실행시키면 빌드가 진행됨을 알리는 로그가 출력됩니다. 빌드가 완료되면 `pong-build` 폴더가 새롭게 생성됩니다. `pong-build` 디렉터리에는 `debug`와 `release` 디렉터리가 새로 생성되어있는데, 각각의 디렉터리에서 debug버전 빌드와 release버전의 빌드를 할 수 있습니다.

## 게임 서버 빌드
`pong-build/debug` 디렉터리로 이동하여 `make`명령을 입력해 봅시다. 다음과 같은 메시지가 출력되었다면 성공입니다.

```bash
$ cd pong-build/debug
$ make

...
Linking CXX shard module libpong.so
[100%] Built target Pong
```

## 게임 서버 실행
iFunEngine은 빌드 후 `-local` 스크립트와 `-launcher` 스크립트를 생성합니다. `-local` 스크립트는 개발 중에 서버를 실행할 때 사용하며 `-launcher` 스크립트는 게임 서버를 패키징하여 데몬으로 실행할 때 사용합니다.

지금은 `pong-local`을 실행하여 테스트 해보겠습니다.

```bash
$ cd pong-build/debug
$ ./pong-local
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

다음으로, 다운받은 Pong Client 프로젝트를 실행시켜 Main씬을 로드합니다. `GameLogic` 오브젝트의 `Multi play`항목을 `true`로 체크하고, `NetworkManager` 오브젝트 의 Server addr값을 현재 서버의 주소로 변경해주세요.

게임을 실행시키고 **[게임시작]** 버튼을 누르면 클라이언트에서 서버로 로그인 요청을 보내게 됩니다. 서버가 요청을 정상적으로 처리하고 클라이언트에게 응답하면 클라이언트에서는 **[대전시작]** 버튼이 활성화되고, 서버에서는 로그인 성공 메시지를 출력함을 확인할 수 있습니다.

```bash
...
I0109 00:00:00.745265  9461 transport.cc:292] Client plugin version: 186
I0109 00:00:00.745446  9461 session_service.cc:1262] session created: 3fceb2f1-ed6c-4cfc-a026-58744521df92
I0109 00:00:00.793865  9462 transport.cc:292] Client plugin version: 186
W0109 00:00:00.793936  9462 session_impl.cc:1006] message with seq number. but seq number validation disabled: sid=3fceb2f1-ed6c-4cfc-a026-58744521df92, transport_protocol=Tcp
I0109 00:00:00.795513  9482 event_handlers.cc:66] login succeed : 4f4ccf9233f6cd83978a5bd21ad41e1e61829d81_Editor
```

활성화된 **[대전시작]** 버튼을 누르면 클라이언트에서 서버로 매치 요청을 보내게 되고, 현재는 클라이언트가 하나뿐이므로 서버에서 일정 시간을 기다린 후 Timeout되어 매칭을 취소하게 됩니다.

```bash
...
I0109 00:00:00.627674  9571 pong_server.cc:46] OnJoined 4f4ccf9233f6cd83978a5bd21ad41e1e61829d81_Editor
I0109 00:00:00.632324  9555 event_handlers.cc:113] OnMatched : Timeout : null
```
