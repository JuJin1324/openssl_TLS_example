//
// Created by Ju-Jin Yoo on 2020/01/07.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>

#define MAX_BUF_SIZE 256

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

void configure_context(SSL_CTX *ctx, char *crt_file, char *private_key_file) {
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, crt_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, private_key_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

BIO *create_SSL_BIO(SSL_CTX *ctx) {
    BIO *bio;
    SSL *ssl;

    bio = BIO_new_ssl(ctx, 0);
    if (bio == NULL) {
        printf("Error to make BIO object.\n");
        exit(EXIT_FAILURE);
    }
    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    return bio;
}

int main(int count, char *strings[]) {
    SSL_CTX *ctx;

    BIO *ssl_bio;       /* SSL Context에서 생성될 메인 BIO 객체 */
    BIO *accept_bio;      /* accept BIO 로써 인커밍 연결을 수락 */
    BIO *rw_bio;       /* 클라이언트와 통신하는 BIO 객체 */

    init_openssl();
    ctx = create_context(SSLv23_server_method());
    /* private.crt 및 private.key 파일은 README.md를 보고 직접 만들어서 해보기를 바랍니다. */
    configure_context(ctx, "../ssl-keys/private.crt", "../ssl-keys/private.key");

    ssl_bio = create_SSL_BIO(ctx);

    accept_bio = BIO_new_accept("43211");
    BIO_set_accept_bios(accept_bio, ssl_bio);

    /* First call to set up for accepting incoming connections... */
    if (BIO_do_accept(accept_bio) <= 0) {
        printf("Error to accept...\n");
        exit(EXIT_FAILURE);
    }
    /* Second call to actually wait */
    if (BIO_do_accept(accept_bio) <= 0) {
        printf("Error to accept...\n");
        exit(EXIT_FAILURE);
    }

    /* 클라이언트와 연결 */
    rw_bio = BIO_pop(accept_bio);
    if (BIO_do_handshake(rw_bio) <= 0) {
        printf("Error to handshake...\n");
        exit(EXIT_FAILURE);
    }

    char buf[MAX_BUF_SIZE];
    int x = BIO_read(rw_bio, buf, MAX_BUF_SIZE);
    if (x == 0) {
        printf("Connection closed.\n");
//        return EXIT_FAILURE;
    } else if (x < 0) {
        if (!BIO_should_retry(rw_bio)) {
            printf("Can't retry BIO_read.\n");
            exit(EXIT_FAILURE);
        }
        /* Do something to handle retry */
        BIO_read(rw_bio, buf, MAX_BUF_SIZE);
    }
    printf("Read from Client : %s\n", buf);

    sleep(1);
    sprintf(buf, "Server : Hello SSL Client.");
    /* 5. 연결 WRITE */
    if (BIO_write(rw_bio, buf, (int) strlen(buf)) <= 0) {
        if (BIO_should_retry(rw_bio)) {
            printf("Can't retry BIO_write\n");
            exit(EXIT_FAILURE);
        }
        /* Do something to handler retry */
        BIO_write(rw_bio, buf, MAX_BUF_SIZE);
    }
    printf("Send to Client : %s\n", buf);

    BIO_free_all(rw_bio);
    BIO_free_all(accept_bio);     /* bio는 abio를 해제하면 자동 해제됨. */
    SSL_CTX_free(ctx);            /* release context */

    return EXIT_SUCCESS;
}