cmake_minimum_required(VERSION 3.22.1)
project(cm)

set(CMAKE_CXX_STANDARD 17)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_definitions("-g -Os -Wall -fPIC -std=c++17")

file(GLOB SRC_FILES
        "${PROJECT_SOURCE_DIR}/cm/base/*"
        "${PROJECT_SOURCE_DIR}/cm/net/*"
        "${PROJECT_SOURCE_DIR}/src/base/*"
        "${PROJECT_SOURCE_DIR}/src/net/*")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(cma STATIC ${SRC_FILES})
add_library(cmo SHARED ${SRC_FILES})

SET_TARGET_PROPERTIES(cma PROPERTIES VERSION 1.1 SO-VERSION 1)
SET_TARGET_PROPERTIES(cmo PROPERTIES VERSION 1.1 SO-VERSION 1)

set_target_properties(cma PROPERTIES OUTPUT_NAME cm)
set_target_properties(cmo PROPERTIES OUTPUT_NAME cm)

install(TARGETS cmo cma LIBRARY DESTINATION lib/cm ARCHIVE DESTINATION lib/cm)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cm/ DESTINATION include/cm)

#add_executable(${CMAKE_PROJECT_NAME} main.cpp ${SRC_FILES})

find_package(Threads)