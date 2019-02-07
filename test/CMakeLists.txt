add_executable(lz4_stream_test lz4_stream_test.cpp)
target_link_libraries(lz4_stream_test ${LZ4_STREAM_LIBRARY_NAME} catch2)
add_test(lz4_stream_test lz4_stream_test)
