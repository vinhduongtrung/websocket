cmake_minimum_required(VERSION 3.24)
project(socketserver)

set(CMAKE_CXX_STANDARD 17)

add_executable(socketserver main.c socketutil.c)
