cmake_minimum_required(VERSION 3.10)
project(lab7 C)

set(CMAKE_C_STANDARD 99)

add_executable(barber1 GeislerWojciech/cw07/zad1/barber.c GeislerWojciech/cw07/zad1/common.c GeislerWojciech/cw07/zad1/common.h)
add_executable(client1 GeislerWojciech/cw07/zad1/client.c GeislerWojciech/cw07/zad1/common.c GeislerWojciech/cw07/zad1/common.h)
add_executable(barber2 GeislerWojciech/cw07/zad2/barber.c GeislerWojciech/cw07/zad2/common.c GeislerWojciech/cw07/zad2/common.h)
add_executable(client2 GeislerWojciech/cw07/zad2/client.c GeislerWojciech/cw07/zad2/common.c GeislerWojciech/cw07/zad2/common.h)
