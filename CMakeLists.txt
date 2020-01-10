cmake_minimum_required(VERSION 3.15)
project(openssl_TLS_example C)

set(CMAKE_C_STANDARD 99)
set(OPENSSL_DIR /usr/local/Cellar/openssl/1.0.2t)

include_directories(
        ${OPENSSL_DIR}/include
)

#add_executable(ssl_client.out
#        src/ssl_client.c
#        )
#target_link_libraries(ssl_client.out -L${OPENSSL_DIR}/lib ssl crypto)
#
#add_executable(ssl_server.out
#        src/ssl_server.c
#        )
#target_link_libraries(ssl_server.out -L${OPENSSL_DIR}/lib ssl crypto)

add_executable(bio_client.out
        src/bio/bio_client.c
        )
target_link_libraries(bio_client.out -L${OPENSSL_DIR}/lib ssl crypto)

add_executable(bio_server.out
        src/bio/bio_server.c
        )
target_link_libraries(bio_server.out -L${OPENSSL_DIR}/lib ssl crypto)

add_executable(ssl_client.out
        src/ssl/ssl_client.c
        )
target_link_libraries(ssl_client.out -L${OPENSSL_DIR}/lib ssl crypto)

add_executable(ssl_server.out
        src/ssl/ssl_server.c
        )
target_link_libraries(ssl_server.out -L${OPENSSL_DIR}/lib ssl crypto)