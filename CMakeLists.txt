cmake_minimum_required(VERSION 3.16)
project(CmfNetLib)

set(CMAKE_CXX_STANDARD 14)

file(GLOB SRC_FILES
        "${PROJECT_SOURCE_DIR}/include/base/*"
        "${PROJECT_SOURCE_DIR}/include/net/*"
        "${PROJECT_SOURCE_DIR}/src/base/*"
        "${PROJECT_SOURCE_DIR}/src/net/*")

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(CmfNet STATIC ${SRC_FILES})
set_target_properties(CmfNet PROPERTIES OUTPUT_NAME CmfNet PREFIX lib SUFFIX .a VERSION ${CMAKE_PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRC_FILES})
find_package(Threads)
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
