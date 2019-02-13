#include <string>
#include <sstream>

#include "catch.hpp"

#include "lz4_stream.h"

TEST_CASE("Lz4Stream") {
  std::stringstream compressed_stream;
  std::stringstream decompressed_stream;

  lz4_stream::ostream lz4_out_stream(compressed_stream);
  lz4_stream::istream lz4_in_stream(compressed_stream);

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
    lz4_out_stream << test_string;
    lz4_out_stream.close();
    decompressed_stream << lz4_in_stream.rdbuf();

    CHECK(decompressed_stream.str() == test_string);
  }

  SECTION("Empty data") {
    lz4_out_stream.close();
    decompressed_stream << lz4_in_stream.rdbuf();

    CHECK(decompressed_stream.str() == "");
  }

  SECTION("All zeroes") {
    lz4_out_stream << std::string(1024, '\0');
    lz4_out_stream.close();
    decompressed_stream << lz4_in_stream.rdbuf();

    CHECK(decompressed_stream.str() == std::string(1024, '\0'));
  }

  SECTION("Small output buffer") {
    std::stringstream compressed_stream;
    lz4_stream::basic_ostream<8> lz4_out_stream(compressed_stream);
    lz4_stream::istream lz4_in_stream(compressed_stream);
    lz4_out_stream << test_string;
    lz4_out_stream.close();
    decompressed_stream << lz4_in_stream.rdbuf();

    CHECK(decompressed_stream.str() == test_string);
  }

  SECTION("Small input buffer") {
    lz4_stream::basic_istream<8, 8> lz4_in_stream(compressed_stream);
    lz4_out_stream << test_string;
    lz4_out_stream.close();
    decompressed_stream << lz4_in_stream.rdbuf();

    CHECK(decompressed_stream.str() == test_string);
  }

  SECTION("Small input and ouput buffer") {
    std::stringstream compressed_stream;
    lz4_stream::basic_istream<8, 8> lz4_in_stream(compressed_stream);
    lz4_stream::basic_ostream<8> lz4_out_stream(compressed_stream);
    lz4_out_stream << test_string;
    lz4_out_stream.close();
    decompressed_stream << lz4_in_stream.rdbuf();

    CHECK(decompressed_stream.str() == test_string);
  }
}
