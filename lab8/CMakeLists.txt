cmake_minimum_required(VERSION 3.10)
project(lab8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -lm)

add_executable(main GeislerWojciech/cw08/zad1/main.c)
