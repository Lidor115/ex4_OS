cmake_minimum_required(VERSION 3.14)
project(ex3 C)

set(CMAKE_C_STANDARD 99)
add_executable(ex3 osqueue.c osqueue.h threadPool.h threadpool.c test.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

