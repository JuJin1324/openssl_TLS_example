//
// Created by Ju-Jin Yoo on 2020/01/08.
//
// 비보안 연결 구축하기
// OpenSSL은 BIO라고 하는 추상화 라이브러리를 사용하여 파일과 소켓을 포함한 다양한 종류의 통신을 보안 또는 비보안 방식으로 처리한다.
// 또한 UU 또는 Base64 코딩용 필터로서도 설정될 수 있다.
//

/* 1. 필수 헤더 : OpenSSL headers */
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BUF_SIZE    256

int main(void) {
    char buf[MAX_BUF_SIZE];
    /* 2. 서버 연결을 위한 포인터 */
    BIO *bio;
    /* 3. 연결 생성 및 시작 */
    bio = BIO_new_accept("43211");
    if (bio == NULL) {
        printf("Error to make BIO object.");
        return EXIT_FAILURE;
    }
    if (BIO_do_accept(bio) <= 0) {
        printf("Error to make accept.");
        return EXIT_FAILURE;
    }

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
        printf("Connection closed.");
//        return EXIT_FAILURE;
    } else if (x < 0) {
        if (!BIO_should_retry(bio)) {
            printf("Can't retry BIO_read.");
            return EXIT_FAILURE;
        }
        /* Do something to handle retry */
        BIO_read(bio, buf, MAX_BUF_SIZE);
    }
    printf("%s\n", buf);

    sleep(1);
    sprintf(buf, "world");
    /* 5. 연결 WRITE */
    if (BIO_write(bio, buf, (int) strlen(buf)) <= 0) {
        if (BIO_should_retry(bio)) {
            printf("Can't retry BIO_write");
            return EXIT_FAILURE;
        }
        /* Do something to handler retry */
        BIO_write(bio, buf, MAX_BUF_SIZE);
    }

    /* 6. 연결 닫기 */
    /* BIO 변수를 재 사용하고 싶으면 다음을 호출 */
    BIO_reset(bio);
    /* bio를 메모리 free */
    BIO_free_all(bio);

    return EXIT_SUCCESS;
}
