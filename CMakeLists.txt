cmake_minimum_required(VERSION 3.7)
project(lz4_stream DESCRIPTION "LZ4 cpp stream." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

add_library(lz4_stream INTERFACE)
target_include_directories(lz4_stream INTERFACE include/)

add_subdirectory(lz4)
target_link_libraries(lz4_stream INTERFACE lz4)
