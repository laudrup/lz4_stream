cmake_minimum_required(VERSION 3.7)
project(lz4_stream DESCRIPTION "LZ4 cpp stream." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

file(GLOB sources "${CMAKE_CURRENT_LIST_DIR}/src/*.c")
add_library(lz4_stream SHARED ${sources})
set_target_properties(lz4_stream PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(lz4_stream PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)
target_include_directories(lz4_stream PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lz4)
target_include_directories(lz4_stream PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
