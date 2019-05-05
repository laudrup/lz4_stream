cmake_minimum_required(VERSION 3.7)
project(lz4 DESCRIPTION "LZ4 compression library." LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)

file(GLOB sources "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")
add_library(lz4 SHARED ${sources})
set_target_properties(lz4 PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(lz4 PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include/>)
target_include_directories(lz4 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

include_directories(lz4 include)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
        -O0 -Wall -Wextra -pedantic -g -Wshadow -Wformat=2 -Wconversion \
        -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC \
        -D_FORTIFY_SOURCE=2 -fsanitize=undefined")

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O2 -funroll-loops -ftree-vectorize")