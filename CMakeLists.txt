cmake_minimum_required(VERSION 3.10)
project(stegobmp C)

set(CMAKE_C_STANDARD 11)

set(SOURCES main.c src/parser.c src/bmp.c)

set(HEADERS include/parser.h include/stegobmp.h include/bmp.h)

add_executable(stegobmp ${SOURCES} ${HEADERS})