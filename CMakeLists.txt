cmake_minimum_required(VERSION 3.28)
project(8085)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "-static")
add_executable(8085 main.cpp)