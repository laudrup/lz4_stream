env = Environment(CXXFLAGS="-std=c++14")

env.Library(target="lz4_stream", source=["lz4_input_stream.cpp",
                                         "lz4_output_stream.cpp"])

env.Program(target="lz4_compress", source=["lz4_compress.cpp"],
            LIBS=["lz4_stream", "lz4"], LIBPATH=".")

env.Program(target="lz4_decompress", source=["lz4_decompress.cpp"],
            LIBS=["lz4_stream", "lz4"], LIBPATH=".")

env.Program(target="lz4_stream_test", source=["lz4_stream_test.cpp"],
            LIBS=["lz4_stream", "lz4", "gtest"], LIBPATH=".")
