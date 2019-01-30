cmake_minimum_required(VERSION 3.1)

# TODO: Remove C language
project(lz4_stream LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LZ4_STREAM_LIBRARY_NAME ${PROJECT_NAME})
set(LZ4_STREAM_LIBRARY_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/")

# TODO: Probably shouldn't be set like this
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

# TODO: Probably shouldn't be set like this
set(CMAKE_CXX_FLAGS_COVERAGE
  "${GCC_DEBUG_FLAGS} -fprofile-arcs -ftest-coverage"
  CACHE STRING "Flags used by the C++ compiler during coverage builds."
  FORCE)
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
  "${CMAKE_EXE_LINKER_FLAGS} --coverage"
  CACHE STRING "Flags used for linking binaries during coverage builds."
  FORCE)
mark_as_advanced(
  CMAKE_CXX_FLAGS_COVERAGE
  CMAKE_EXE_LINKER_FLAGS_COVERAGE)

# TODO: Use external project
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lz4/lib/lz4.h")
  set(LZ4_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/lz4)
  include_directories(${LZ4_ROOT}/lib)
  file(GLOB LZ4_SRC
    "${LZ4_ROOT}/lib/*.h"
    "${LZ4_ROOT}/lib/*.c"
    )
  add_library(LZ4_LIBRARY STATIC ${LZ4_SRC})
else()
  find_package(LZ4 REQUIRED)
endif()

# Create lz4_stream library
add_library(${LZ4_STREAM_LIBRARY_NAME} INTERFACE)
target_compile_features(${LZ4_STREAM_LIBRARY_NAME} INTERFACE cxx_std_11)
target_include_directories(${LZ4_STREAM_LIBRARY_NAME} INTERFACE ${LZ4_STREAM_LIBRARY_INCLUDE_DIR})
target_link_libraries(${LZ4_STREAM_LIBRARY_NAME} INTERFACE ${LZ4_LIBRARY})

# TODO: Use external project
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/googletest/CMakeLists.txt")
  set(GTEST_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/googletest)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  add_subdirectory(${GTEST_ROOT})
  enable_testing()

  add_executable(lz4_stream_test lz4_stream_test.cpp)
  include_directories(lz4_stream_test ${GTEST_ROOT}/googletest/include)
  target_link_libraries(lz4_stream_test gtest ${LZ4_STREAM_LIBRARY_NAME})

  if(NOT MSVC)
    # TODO: Use thread package
    find_package(Threads REQUIRED)
    target_link_libraries(lz4_stream_test pthread)
  endif()

  add_test(lz4_stream_test lz4_stream_test)
else()
  message(STATUS "googletest submodule not found. Tests disabled.")
endif()

# Create test executables
# TODO: Move to own directory
add_executable(lz4_compress lz4_compress.cpp)
target_link_libraries(lz4_compress ${LZ4_STREAM_LIBRARY_NAME})

add_executable(lz4_decompress lz4_decompress.cpp)
target_link_libraries(lz4_decompress ${LZ4_STREAM_LIBRARY_NAME})
