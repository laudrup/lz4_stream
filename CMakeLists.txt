cmake_minimum_required(VERSION 3.7)
project(lz4_stream DESCRIPTION "LZ4 cpp stream." LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(lz4_stream lz4_stream.hpp)
set_target_properties(lz4_stream PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(lz4)
target_link_libraries(lz4_stream lz4)

include_directories(${CMAKE_CURRENT_LIST_DIR})