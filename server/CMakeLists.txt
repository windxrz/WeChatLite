cmake_minimum_required(VERSION 3.8)
project(server)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Actions.hpp Actions.cpp Server.cpp Server.hpp)
add_executable(server ${SOURCE_FILES})