cmake_minimum_required(VERSION 3.22)
project(muduo)

set(CMAKE_CXX_STANDARD 14)

add_definitions("-g -O2 -Wall -fPIC")

file(GLOB SRC_FILES 
    "${PROJECT_SOURCE_DIR}/src/*")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRC_FILES})

find_package(Threads)