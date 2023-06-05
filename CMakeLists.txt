cmake_minimum_required(VERSION 3.22)
project(cm)

set(CMAKE_CXX_STANDARD 14)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_definitions("-g -O2 -Wall -fPIC -std=c++14")

file(GLOB SRC_FILES
        "${PROJECT_SOURCE_DIR}/include/base/*"
        "${PROJECT_SOURCE_DIR}/include/net/*"
        "${PROJECT_SOURCE_DIR}/src/base/*"
        "${PROJECT_SOURCE_DIR}/src/net/*")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(cm STATIC ${SRC_FILES})

#add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRC_FILES})

find_package(Threads)