cmake_minimum_required(VERSION 3.9)
project(lab2 C)

set(CMAKE_C_STANDARD 99)

set(FLAGS "-Wall -Wextra -fsanitize=address -fsanitize=undefined -fsanitize=leak \
    -lasan -lubsan -Wint-in-bool-context")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${FLAGS}" )

add_executable(zad1 GeislerWojciech/cw03/zad1/main.c)

add_executable(zad2  GeislerWojciech/cw03/zad2/main.c)

add_executable(zad3 GeislerWojciech/cw03/zad3a/main.c)

add_executable(big GeislerWojciech/cw03/zad3b/big.c)
add_executable(long GeislerWojciech/cw03/zad3b/long.c)
