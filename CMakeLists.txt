cmake_minimum_required(VERSION 3.16)
project(CmfNetLib)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SRC_FILES
        "${PROJECT_SOURCE_DIR}/NetLib/log/*.h"
        "${PROJECT_SOURCE_DIR}/NetLib/log/*.cpp")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRC_FILES})
