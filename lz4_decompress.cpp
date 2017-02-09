// Own headers
#include "lz4_stream.h"

// Standard headers
#include <algorithm>
#include <fstream>
#include <iostream>

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " [inputfile] [outputfile]" << std::endl;
  }
  std::ifstream in_file(argv[1]);
  std::ofstream out_file(argv[2]);

  LZ4InputStream lz4_stream(in_file);

  std::copy(std::istreambuf_iterator<char>(lz4_stream),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(out_file));

  return 0;
}
