cmake_minimum_required(VERSION 3.1)
project(lz4_stream)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib/lz4.h")
  set(LZ4_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lz4)
  include_directories(${LZ4_ROOT}/lib)
  file(GLOB LZ4_SRC
    "${LZ4_ROOT}/lib/*.h"
    "${LZ4_ROOT}/lib/*.c"
    )
  add_library(lz4 STATIC ${LZ4_SRC})
else()
  find_package(LZ4 REQUIRED)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")
  set(GTEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
  set(gtest_force_shared_crt ON)
  add_subdirectory(${GTEST_ROOT})
  enable_testing()

  add_executable(lz4_stream_test lz4_stream_test.cpp)
  include_directories(lz4_stream_test ${GTEST_ROOT}/googletest/include)
  target_link_libraries(lz4_stream_test lz4_stream gtest)

  if(NOT MSVC)
    find_package(Threads REQUIRED)
    target_link_libraries(lz4_stream_test pthread)
  endif()

  add_test(lz4_stream_test lz4_stream_test)
else()
  message(STATUS "googletest submodule not found. Tests disabled.")
endif()

add_library(lz4_stream STATIC lz4_input_stream.cpp lz4_output_stream.cpp)
target_link_libraries(lz4_stream lz4)

add_executable(lz4_compress lz4_compress.cpp)
target_link_libraries(lz4_compress lz4_stream)

add_executable(lz4_decompress lz4_decompress.cpp)
target_link_libraries(lz4_decompress lz4_stream)
