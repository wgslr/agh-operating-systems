cmake_minimum_required(VERSION 3.9)
project(task C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FALS} -Wall -Wextra")

add_executable(task GeislerWojciech/cw01/zad1/chararray.h GeislerWojciech/cw01/zad1/chararray.c GeislerWojciech/cw01/zad2/main.c)