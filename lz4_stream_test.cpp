// Own headers
#include "lz4_stream.h"

// Google test headers
#include <gtest/gtest.h>

// Standard headers
#include <random>
#include <algorithm>
#include <climits>
#include <iostream>
#include <sstream>

namespace
{
  size_t random_number(size_t max_size)
  {
    std::random_device rd;
    std::mt19937 mt(rd());
    return std::uniform_int_distribution<size_t>(0, max_size)(mt);
  }

  std::string random_string(size_t size)
  {
    using random_chars_engine = std::independent_bits_engine<
      std::default_random_engine, CHAR_BIT, unsigned char>;

    random_chars_engine rbe;
    std::string str;
    str.reserve(size);
    std::generate_n(std::back_inserter(str), size, std::ref(rbe));

    return str;
  }

  std::string compress_decompress_string(const std::string& input_string)
  {
    std::stringstream input_stream(input_string);
    std::stringstream compressed_stream;

    LZ4OutputStream lz4_out_stream(compressed_stream);

    std::copy(std::istreambuf_iterator<char>(input_stream),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(lz4_out_stream));
    lz4_out_stream.close();

    LZ4InputStream lz4_in_stream(compressed_stream);
    std::stringstream decompressed_stream;

    std::copy(std::istreambuf_iterator<char>(lz4_in_stream),
              std::istreambuf_iterator<char>(),
              std::ostreambuf_iterator<char>(decompressed_stream));

    return decompressed_stream.str();
  }
}

TEST(LZ4Stream, largeRandomString)
{
  const size_t max_size = 10 * 1024 * 1024;
  size_t string_size = random_number(max_size);
  std::string test_string = random_string(string_size);

  EXPECT_EQ(test_string, compress_decompress_string(test_string));
}

TEST(LZ4Stream, emptyString)
{
  std::string test_string;

  EXPECT_EQ(test_string, compress_decompress_string(test_string));
}

TEST(LZ4Stream, blockSizeBoundary)
{
  const size_t boundary_size = 64 * 1024;
  std::string test_string = random_string(boundary_size);

  EXPECT_EQ(test_string, compress_decompress_string(test_string));
}

TEST(LZ4Stream, allZeroes)
{
  size_t size = random_number(1024 * 1024);
  std::string test_string(size, '\0');

  EXPECT_EQ(test_string, compress_decompress_string(test_string));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
