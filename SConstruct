import os

CXX = os.getenv("CXX", "g++")

CPPPATH = []
if "CPPPATH" in os.environ:
    CPPPATH = [os.getenv("CPPPATH")]

LIBPATH = ["."]
if "LIBPATH" in os.environ:
    LIBPATH += [os.getenv("LIBPATH")]

if CXX in ["g++", "clang++"]:
    CXXFLAGS = ["-std=c++14", "-Wall", "-Wextra", "-Werror"]
    LIBS = [File("liblz4_stream.a"), "lz4"]
elif CXX in ["cl", "cl.exe"]:
    CXXFLAGS = ["-W4", "-EHsc"]
    LIBS = [File("lz4_stream.lib"), File("liblz4_static.lib")]
else:
    CXXFLAGS = []

LIBRARY_SOURCES = ["lz4_input_stream.cpp", "lz4_output_stream.cpp"]

env = Environment(CXXFLAGS=CXXFLAGS, CXX=CXX, CPPPATH=CPPPATH)

env.Library(target="lz4_stream", source=LIBRARY_SOURCES)

env.SharedLibrary(target="lz4_stream", source=["lz4_input_stream.cpp",
                                               "lz4_output_stream.cpp"])

env.Program(target="lz4_compress", source=["lz4_compress.cpp"],
            LIBS=LIBS, LIBPATH=LIBPATH)

env.Program(target="lz4_decompress", source=["lz4_decompress.cpp"],
            LIBS=LIBS, LIBPATH=LIBPATH)

env.Program(target="lz4_stream_test", source=["lz4_stream_test.cpp"],
            LIBS=LIBS + ["lz4", "gtest", "pthread"], LIBPATH=LIBPATH)
