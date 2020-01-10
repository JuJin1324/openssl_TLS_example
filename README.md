# openssl-TLS-example

## SSL
### SSL 이란?
SSL은 Secure Sockets Layer의 약자이다. 인터넷 상의 보안 통신 표준이고, 데이터 암호화와 프로토콜을 통합한다. 
데이터는 컴퓨터를 떠나기 전에 암호화 되고, 목적지에 도착하면 암호가 해독된다. 
인증과 암호 알고리즘이 뒤를 받치고 있고, OpenSSL에서는 이 두 가지 모두를 사용할 수 있다.

### SSL 디지털 인증서
SSL 인증서는 클라이언트와 서버간의 통신을 제3자가 보증해주는 전자화된 문서다. 
클라이언트가 서버에 접속한 직후에 서버는 클라이언트에게 이 인증서 정보를 전달한다. 
클라이언트는 이 인증서 정보가 신뢰할 수 있는 것인지를 검증 한 후에 다음 절차를 수행하게 된다.

### CA
인증서의 역할은 클라이언트가 접속한 서버가 클라이언트가 의도한 서버가 맞는지를 보장하는 역할을 한다. 
이 역할을 하는 민간기업들이 있는데 이런 기업들을 CA(Certificate authority) 혹은 Root Certificate 라고 부른다. 
CA는 아무 기업이나 할 수 있는 것이 아니고 신뢰성이 엄격하게 공인된 기업들만이 참여할 수 있다.

### Client to Server SSL 통신 과정
* Client -> Server로 Client Hello 보냄
* Server -> Client 로 Server Hello 와 인증서를 보냄
* Client 에서 Server로 부터 받은 인증서의 CA 정보가 Client의 CA 리스트에 있는지 확인 후 리스트에 있으면 계속 통신 / 없으면 통신 해제
* 인증서 안에 Server의 공개키(public key)가 같이 들어있음. Client와 Server는 대칭키를 만들기 위해서 Server가 가진 개인키(private key) 와
Client가 서버로 부터 전달받은 공개키(public key)를 통하여 서로 암호화 하여 대칭키를 만들기 위한 데이터를 주고 받아서 대칭키를 만듬.

### 참고자료
[생활코딩 - HTTPS와 SSL 인증서](https://opentutorials.org/course/228/4894)

## openssl
* 설명 : 전자상거래 방식의 세계 표준으로 사용되는 안전 전송규약으로 전송시 암호화하는 SSL (Secure Socket Layer), 
전송 계층 보안 TTL(Transfer Layer Security)를 구현한 공개 소프트웨어. C언어 라이브러리 포함되어있음. 
* 설치 
    - macOS : `brew install openssl`
    - Ubuntu : 선탑재

### BIO
OpenSSL에서 파일과 소켓을 포함한 다양한 종류의 통신을 보안 또는 비보안 방식으로 처리하는 추상화 라이브러리.
* 파일 I/O 및 소켓 I/O 모두 처리가능한 라이브러리
* SSL 보안 혹은 비보안 방식으로 선택하여 통신 가능

## 인증서 발급
openssl을 사용하여 테스트를 위한 사설 인증서를 발급한다. 터미널 작업이며 아래 명령어들은 터미널에서 `openssl`로 openssl 접속 이후에 실행하거나
앞에 openssl 접속하지 않고 사용할 시에 명령어 앞에 `openssl`을 붙여서 실행한다.

### 1. 개인키, 공개키 발급
* pass phrase가 없는 개인키 발급 : genrsa -out [키이름] 2048 
    - 예시 : `genrsa -out private.key 2048`
* 공개키 발급 : genrsa -in [개인키] -pubout -out public.key
    - 예시 : `genrsa -in private.key -pubout -out public.key`

### 2. CSR(인증 신청서) 만들기
CSR : SSL 인증의 정보를 암호화하여 인증기관에 보내 인증서를 발급받게하는 신청서로 생성 시에 국가코드, 도시, 회사명, 부서명, 이메일, 도메인주소 등을 기입해야함.
* CSR(인증요청서) 생성 : req -new -key [키] -out [CSR 이름]
    - 예시 : `req -new -key private.key -out private.csr`

### 3. 사설 rootCA CSR 만들기
사설 CA 인증을 위한 CA 생성
* rootCA CSR을 생성하기 위한 key 생성 : genrsa [암호화 알고리즘] -out [키 이름] 2048
    - 예시 : `genrsa -aes256 -out rootCA.key 2048`
* 사설 rootCA CSR 생성 : req -x509 -new -nodes -key [rootCA용으로 만든 key] -days [인증서 유효 일수] -out [CSR 이름]
    - 생성 시에 국가코드, 도시, 회사명, 부서명, 이메일, 도메인주소 등을 기입해야함.
    - 예시(인증서 유효 일수 10년으로 지정하였음) : `req -x509 -new -nodes -key rootCA.key -days 3650 -out rootCA.pem`
    
### 4. CRT 만들기
CRT : 인증서
* 10년짜리 인증서 생성 : `x509 -req -in private.csr -CA rootCA.pem -CAkey rootCA.key -CAcreateserial -out private.crt -days 3650`

### 참고자료
* [SSL - HTTPS통신을 위한 SSL인증서 발급하기(OpenSSL)](https://namjackson.tistory.com/24)
* [OpenSSL API를 이용한 보안 프로그래밍](http://hasu0707.nflint.com/xe/index.php?mid=development_lib&document_srl=382)
* [리눅스 공인인증 경로 찾기](https://serverfault.com/questions/62496/ssl-certificate-location-on-unix-linux)
* [SSL Programming Tutorial](https://tribal1012.tistory.com/213)
