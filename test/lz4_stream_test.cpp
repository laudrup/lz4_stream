#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include <string>

#include "lz4_stream.h"

namespace
{
std::string compress_decompress_string(const std::string& input_string)
{
  std::stringstream input_stream(input_string);
  std::stringstream compressed_stream;

  lz4_stream::ostream lz4_out_stream(compressed_stream);

  std::copy(std::istreambuf_iterator<char>(input_stream),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(lz4_out_stream));
  lz4_out_stream.close();

  lz4_stream::istream lz4_in_stream(compressed_stream);
  std::stringstream decompressed_stream;

  std::copy(std::istreambuf_iterator<char>(lz4_in_stream),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(decompressed_stream));

  return decompressed_stream.str();
}
}

TEST_CASE("Lz4Stream") {
  std::string test_string =
    "Three Rings for the Elven-kings under the sky,\n"
    "Seven for the Dwarf-lords in their halls of stone,\n"
    "Nine for Mortal Men doomed to die,\n"
    "One for the Dark Lord on his dark throne\n"
    "In the Land of Mordor where the Shadows lie.\n"
    "One Ring to rule them all, One Ring to find them,\n"
    "One Ring to bring them all, and in the darkness bind them,\n"
    "In the Land of Mordor where the Shadows lie.\n";

  SECTION("Default compression/decompression") {
    CHECK(compress_decompress_string(test_string) == test_string);
  }

  SECTION("Empty data") {
    CHECK(compress_decompress_string("") == "");
  }

  SECTION("All zeroes") {
    std::string zero_string(1024, '\0');
    CHECK(compress_decompress_string(zero_string) == zero_string);
  }
}
