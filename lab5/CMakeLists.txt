cmake_minimum_required(VERSION 3.10)
project(lab5 C)

set(CMAKE_C_STANDARD 99)

add_executable(zad1 GeislerWojciech/cw05/zad1/main.c)
add_executable(master GeislerWojciech/cw05/zad2/master.c)
add_executable(slave GeislerWojciech/cw05/zad2/slave.c)
