cmake_minimum_required(VERSION 3.10)
project(stegobmp C)

set(CMAKE_C_STANDARD 11)

set(SOURCES main.c src/parser/parser.c src/stegobmp/stegobmp.c src/stegobmp/stegobmp_hide.c src/stegobmp/stegobmp_utils.c src/bmp/bmp.c src/bmp/bmp_utils.c)

set(HEADERS include/parser/parser.h include/stegobmp/stegobmp.h include/stegobmp/stegobmp_hide.h include/stegobmp/stegobmp_utils.h include/bmp/bmp.h include/bmp/bmp_utils.h)

add_executable(stegobmp ${SOURCES} ${HEADERS})