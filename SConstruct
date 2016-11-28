import os

CXX = os.getenv("CXX", "g++")
CXXFLAGS = ["-Wall"]
if CXX in ["g++", "clang++"]:
    CXXFLAGS += ["-std=c++14", "-Wextra", "-Werror"]

LIBRARY_SOURCES = ["lz4_input_stream.cpp", "lz4_output_stream.cpp"]
LIBS = [File("liblz4_stream.a"), "lz4"]

env = Environment(CXXFLAGS=CXXFLAGS, CXX=CXX)

env.Library(target="lz4_stream", source=LIBRARY_SOURCES)

env.SharedLibrary(target="lz4_stream", source=["lz4_input_stream.cpp",
                                               "lz4_output_stream.cpp"])

env.Program(target="lz4_compress", source=["lz4_compress.cpp"],
            LIBS=LIBS, LIBPATH=".")

env.Program(target="lz4_decompress", source=["lz4_decompress.cpp"],
            LIBS=LIBS, LIBPATH=".")

env.Program(target="lz4_stream_test", source=["lz4_stream_test.cpp"],
            LIBS=LIBS + ["lz4", "gtest", "pthread"], LIBPATH=".")
