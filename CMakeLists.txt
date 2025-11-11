cmake_minimum_required(VERSION 3.10)
project(stegobmp C)

set(CMAKE_C_STANDARD 11)

find_package(OpenSSL REQUIRED)

set(SOURCES
        main.c
        src/parser/parser.c
        src/analysis/stego_analysis.c
        src/stegobmp/stegobmp.c
        src/stegobmp/stegobmp_lsb.c
        src/stegobmp/stegobmp_utils.c
        src/bmp/bmp.c
        src/bmp/bmp_utils.c
        src/crypto/crypto.c
)

set(HEADERS
        include/parser/parser.h
        include/analysis/stego_analysis.h
        include/stegobmp/stegobmp.h
        include/stegobmp/stegobmp_lsb.h
        include/stegobmp/stegobmp_utils.h
        include/bmp/bmp.h
        include/bmp/bmp_utils.h
        include/crypto/crypto.h
)

add_executable(stegobmp ${SOURCES} ${HEADERS})

target_link_libraries(stegobmp OpenSSL::Crypto)