import os

CXX = os.getenv("CXX", "g++")

CPPPATH = []
if "CPPPATH" in os.environ:
    CPPPATH = os.getenv("CPPPATH").split(";")

LIBPATH = ["."]

LIBRARY_SOURCES = ["lz4_input_stream.cpp", "lz4_output_stream.cpp"]
STATIC_LIB = "lz4_stream"
UNITTEST_LIBS = ["gtest"]

if CXX in ["g++", "clang++"]:
    CXXFLAGS = ["-std=c++14", "-Wall", "-Wextra", "-Werror"]
    LIBS = [File("lib%s.a" % STATIC_LIB), "lz4"]
    UNITTEST_LIBS += ["pthread"]
elif CXX in ["cl", "cl.exe"]:
    STATIC_LIB += "_static"
    CXXFLAGS = ["-W4", "-EHsc", "-MTd"]
    LIBS = [File("%s.lib" % STATIC_LIB), File("liblz4.lib")]
    LIBRARY_SOURCES += ["liblz4.lib"]
else:
    CXXFLAGS = []

env = Environment(CXXFLAGS=CXXFLAGS,
                  CXX=CXX,
                  CPPPATH=CPPPATH,
                  TARGET_ARCH="x86")

env.StaticLibrary(target=STATIC_LIB, source=LIBRARY_SOURCES)

env.SharedLibrary(target="lz4_stream", source=LIBRARY_SOURCES)

env.Program(target="lz4_compress", source=["lz4_compress.cpp"],
            LIBS=LIBS, LIBPATH=LIBPATH)

env.Program(target="lz4_decompress", source=["lz4_decompress.cpp"],
            LIBS=LIBS, LIBPATH=LIBPATH)

env.Program(target="lz4_stream_test", source=["lz4_stream_test.cpp"],
            LIBS=LIBS + UNITTEST_LIBS, LIBPATH=LIBPATH)
