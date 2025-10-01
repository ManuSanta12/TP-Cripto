cmake_minimum_required(VERSION 3.10)
project(stegobmp C)

set(CMAKE_C_STANDARD 11)

set(SOURCES main.c parser.c)

set(HEADERS parser.h stegobmp.h bmp.h)

add_executable(stegobmp ${SOURCES} ${HEADERS})