cmake_minimum_required(VERSION 3.10)
project(TestShellApp)

set(CMAKE_CXX_STANDARD 17)

add_executable(shell main.cpp testShell.cpp)