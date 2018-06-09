cmake_minimum_required(VERSION 3.10)
project(lab9 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-lm -lpthread -lrt")

add_executable(zad1 GeislerWojciech/cw09/zad1/main.c)
add_executable(zad2 GeislerWojciech/cw09/zad2/main.c)
