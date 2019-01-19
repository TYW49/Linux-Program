cmake_minimum_required(VERSION 3.9)
project(back C)

set(CMAKE_C_STANDARD 99)

add_executable(back main.c communication.c communication.h)