cmake_minimum_required(VERSION 3.16)
project(CmfNetLib)

set(CMAKE_CXX_STANDARD 14)

file(GLOB SRC_FILES
        "${PROJECT_SOURCE_DIR}/NetLib/base/*.hpp"
        "${PROJECT_SOURCE_DIR}/NetLib/log/*.hpp"
        "${PROJECT_SOURCE_DIR}/NetLib/net/*.hpp"
        "${PROJECT_SOURCE_DIR}/NetLib/thread/*.hpp")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRC_FILES})
find_package(Threads)
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
