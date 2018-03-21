cmake_minimum_required(VERSION 3.9)
project(lab2 C)

set(CMAKE_C_STANDARD 99)

set(FLAGS "-Wall -Wextra -fsanitize=address -fsanitize=undefined -fsanitize=leak \
    -lasan -lubsan -Wint-in-bool-context -O2")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${FLAGS}" )

add_executable(zad1 GeislerWojciech/cw02/zad1/main.c)

add_executable(zad2 GeislerWojciech/cw02/zad2/main.c)
