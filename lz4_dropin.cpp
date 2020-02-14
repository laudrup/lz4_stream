// Own headers
#include "lz4_stream.h"

// Standard headers
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cstring>

void die_usage(char* argv[])
{
  using namespace std;
  cerr << "Usage: " << argv[0] << " <-c|-d> <-|INPUT-FILE> <-|OUTPUT-FILE>" << endl;
  exit(1);
}

int main(int argc, char* argv[])
{
  using namespace std;

  if (argc != 4) {
    die_usage(argv);
  }

  string mode = argv[1];
  string input = argv[2];
  string output = argv[3];

  // Parse mode.
  bool compress = true;
  if (mode == "-c") {
  } else if (mode == "-d") {
    compress = false;
  } else {
    die_usage(argv);
  }

  // Parse input file.
  ifstream in_file_stream;
  if (input != "") {
    in_file_stream = ifstream(input);
  }
  istream& in = (input != "-") ? in_file_stream : cin;

  // Parse output file.
  ofstream out_file_stream;
  if (output != "") {
    out_file_stream = ofstream(output);
  }
  ostream& out = (output != "-") ? out_file_stream : cout;

  // Enable exceptions
  in.exceptions(std::istream::badbit);
  out.exceptions(std::ostream::badbit);

  // Run
  if (compress) {

    lz4_stream::ostream lz4_stream(out);

    std::copy(std::istreambuf_iterator<char>(in),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(lz4_stream));

  } else {

    lz4_stream::istream lz4_stream(in);

    std::copy(std::istreambuf_iterator<char>(lz4_stream),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(out));

  }

  return 0;
}
