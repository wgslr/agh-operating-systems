cmake_minimum_required(VERSION 3.10)
project(lab6 C)

set(CMAKE_C_FLAGS -lrt)

set(CMAKE_C_STANDARD 99)

add_executable(server1 GeislerWojciech/cw06/zad1/server.c)
add_executable(client1 GeislerWojciech/cw06/zad1/client.c)

add_executable(server2 GeislerWojciech/cw06/zad2/server.c)
add_executable(client2 GeislerWojciech/cw06/zad2/client.c)
