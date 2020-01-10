//
// Created by Ju-Jin Yoo on 2020/01/08.
//

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdlib.h>

#define MAX_BUF_SIZE    256

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context(const SSL_METHOD *method) {
    SSL_CTX *ctx;

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

/*
 * Set the key and cert
 * 사설 인증서의 경우 crt_file이 필요하며 crt_path는 NULL을 넣어주면 된다.
 * 공인 인증서의 경우 crt_file은 NULL, crt_path에 공인 인증 파일이 위치한 경로를 준다.
 */
void configure_context(SSL_CTX *ctx, char *cert_file, char *cert_path) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (!SSL_CTX_load_verify_locations(ctx, cert_file, cert_path)) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    /*
     * 1. SSL 포인터 설정하기
     * 보안 연결 설정하기
     * 보안 연결을 설정하기 위해 서는 두 줄 이상의 코드가 필요하다. 또 다른 포인터가 SSL_CTX 유형에 필요하다.
     * 이것은 SSL 정보를 포함하고 있는 구조이다. BIO 라이브러리를 통해 SSL 연결을 구축하는데도 사용된다.
     * 이 구조는 SSL 메소드 함수, SSLv23_client_method와 SSL_CTX_new를 호출함으로써 생성된다.
     * 또 다른 포인터 유형의 SSL 역시 SSL 연결 구조를 보유하는데 필요하다.
     * SSL 포인터는 나중에 사용되어 연결 정보를 검사하거나 추가 SSL 매개변수를 설정한다.
     */
    SSL_CTX *ctx;
    SSL *ssl;

    init_openssl();
    ctx = create_context(SSLv23_client_method());
    /*
     * private.crt 파일은 README.md를 보고 직접 만들어서 해보기를 바랍니다.
     * 현재 코드는 사설 인증서를 사용하기 때문에 공인 인증 path는 NULL 두었다.
     * 공인 인증을 통해 SSL 통신을 하려는 경우 cert_file에 NULL을 cert_path에는 해당 공인 인증이 존재하는 경로를 문자열로 적어준다.
     */
    configure_context(ctx, "../ssl-keys/private.crt", NULL);

    /*
     * 4. BIO 객체 설정하기
     * BIO 객체는 BIO_new_ssl_connect를 사용하여 생성된다. 유일한 매개변수로서 SSL 콘텍스트에 대한 포인터를 취한다. SSL 구조에 대한 포인터는 검색되어야 한다.
     * 이 글에서, 이 포인터는 SSL_set_mode 함수로만 사용된다. 이 함수는 SSL_MODE_AUTO_RETRY 플래그를 설정하는데 사용된다.
     * 이 옵션을 설정한 상태에서, 서버가 갑자기 새로운 핸드쉐이크를 원하면, OpenSSL은 이를 뒤에서 핸들한다.
     * 이 옵션이 없이, 읽기 또는 쓰기 연산은 서버가 새로운 핸드쉐이크를 원할 경우 에러를 리턴하면서, 프로세스에서 재시도 플래그를 설정한다.
     */
    BIO *bio = BIO_new_ssl_connect(ctx);
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /*
     * 5. 보안 연결 개방하기
     * SSL 콘텍스트 구조가 설정된 상태에서 연결이 생성될 수 있다. 호스트네임은 BIO_set_conn_hostname 함수를 사용하여 설정된다.
     * 호스트네임과 포트는 위와 같은 포맷으로 지정된다. 이 함수는 또한 호스트로 연결을 개방한다.
     * BIO_do_connect로의 호출 역시 수행되어 연결이 성공적으로 열렸는지를 확인한다.
     * 이와 똑 같은 호출이 핸드쉐이크를 수행하여 보안 통신을 설정한다.
     */
    BIO_set_conn_hostname(bio, "localhost:43211");
    if (BIO_do_connect(bio) <= 0) {
        printf("Error to make SSL connect.\n");
        return EXIT_FAILURE;
    }

    /*
     * 6. 인증서 유효성 여부 검사하기
     * 연결이 이루어지면, 이것이 유효한지 여부를 확인하기 위해 인증서가 검사된다. 실제로는 OpenSSL이 이를 수행한다.
     * 인증서에 치명적인 문제가 있다면(이를 테면, 해시 값이 무효한 경우), 연결이 이루어지지 않는다.
     * 그러나, 그렇게 중요하지 않은 문제가 있다면(만기 또는 무효) 연결이 여전히 사용될 수 있다.
     * OpenSSL에서 인증서가 제대로 되었는지를 확인하려면, SSL_get_verify_result를 호출한다.
     * OpenSSL의 내부로 전달된 인증서가 확인되면(트러스트 체크 포함) X509_V_OK가 리턴된다.
     * 잘못되었다면, 명령행 툴용 verify 옵션 밑에 문서화 된 에러 코드를 리턴한다. 실패했다고 해서 연결이 사용될 수 없는 것은 아니다.
     * 연결이 사용되는지의 여부는 확인 결과와 보안 고려 사항에 의존한다.
     * 예를 들어, 실패한 트러스트 확인은 트러스트 인증서를 사용할 수 없다는 것을 의미한다. 더 강화된 보안으로 연결은 지속된다.
     */
    if (SSL_get_verify_result(ssl) != X509_V_OK) {
        printf("Certificate is not correct!\n");
        return EXIT_FAILURE;
    }

    char buf[MAX_BUF_SIZE];
    /* 5. 연결 WRITE */
    sprintf(buf, "Client : hello SSL Server.");
    if (BIO_write(bio, buf, (int) strlen(buf)) <= 0) {
        if (BIO_should_retry(bio)) {
            printf("Can't retry BIO_write\n");
            return EXIT_FAILURE;
        }
        /* Do something to handler retry */
        BIO_write(bio, buf, MAX_BUF_SIZE);
    }
    printf("Send to Server : %s\n", buf);

    /*
     * 4. 연결에서 읽기
     * 소켓이든 파일이든 상관없이, BIO 객체를 읽고 쓰는 것은 두 개의 함수, BIO_read와 BIO_write를 사용하여 수행된다. 간단하지 않은가?
     * BIO_read는 서버에서 특정 숫자를 읽는다.
     * 바이트가 읽는 숫자, 또는 0 또는-1을 리턴한다.
     * Blocking connection에서, 0이 리턴되었다는 것은 연결이 닫혔다는 것을 의미하고, -1은 에러가 발생했다는 것을 의미한다.
     * Non-blocking connection에서, 0의 리턴은 어떤 데이터도 사용할 수 없음을 나타내고, -1은 에러를 나타낸다.
     * 에러가 복구 가능한 것인지 여부를 파악하려면 BIO_should_retry를 호출한다.
     */
    int x = BIO_read(bio, buf, MAX_BUF_SIZE);
    if (x == 0) {
        printf("Connection closed.\n");
//        return EXIT_FAILURE;
    } else if (x < 0) {
        if (!BIO_should_retry(bio)) {
            printf("Can't retry BIO_read.\n");
            return EXIT_FAILURE;
        }
        /* Do something to handle retry */
        BIO_read(bio, buf, MAX_BUF_SIZE);
    }
    printf("Received from Server : %s\n", buf);

    /* 5. 컨텍스트 제거 */
    BIO_free_all(bio);
    SSL_CTX_free(ctx);

    return EXIT_SUCCESS;
}
