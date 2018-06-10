cmake_minimum_required(VERSION 3.10)
project(lab10 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-lrt -lpthread -lm")

add_executable(server1 GeislerWojciech/zad1/server.c GeislerWojciech/zad1/common.h)
add_executable(client1 GeislerWojciech/zad1/client.c GeislerWojciech/zad1/common.h)
