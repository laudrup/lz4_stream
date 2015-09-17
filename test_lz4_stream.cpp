#include "lz4frame.h"

#include <streambuf>
#include <ostream>
#include <vector>
#include <cassert>
#include <fstream>
#include <memory>
#include <iostream>

class LZ4InputBuffer : public std::streambuf
{
public:
  LZ4InputBuffer(std::ostream &sink);
  ~LZ4InputBuffer();

  LZ4InputBuffer(const LZ4InputBuffer &) = delete;
  LZ4InputBuffer& operator= (const LZ4InputBuffer &) = delete;

private:
  int_type overflow(int_type ch) override;
  int sync() override;

  void compressAndWrite();
  void writeHeader();
  void writeFooter();

  std::ostream& sink_;
  std::array<char, 256> src_buf_;
  std::vector<char> dest_buf_;
  LZ4F_compressionContext_t ctx_;
};

LZ4InputBuffer::LZ4InputBuffer(std::ostream &sink)
  : sink_(sink),
    dest_buf_(LZ4F_compressBound(src_buf_.size(), NULL))
{
  char* base = &src_buf_.front();
  setp(base, base + src_buf_.size() - 1);

  size_t ret = LZ4F_createCompressionContext(&ctx_, LZ4F_VERSION);
  if (LZ4F_isError(ret))
  {
    printf("Failed to create context: error %zu", ret);
  }
  writeHeader();
}

LZ4InputBuffer::~LZ4InputBuffer()
{
  sync();
  writeFooter();
  LZ4F_freeCompressionContext(ctx_);
}

LZ4InputBuffer::int_type LZ4InputBuffer::overflow(int_type ch)
{
  assert(std::less_equal<char*>()(pptr(), epptr()));

  *pptr() = ch;
  pbump(1);

  compressAndWrite();

  return ch;
}

int LZ4InputBuffer::sync()
{
  compressAndWrite();
  return 0;
}

void LZ4InputBuffer::compressAndWrite()
{
  std::ptrdiff_t orig_size = pptr() - pbase();
  pbump(-orig_size);
  size_t comp_size = LZ4F_compressUpdate(ctx_, &dest_buf_.front(), dest_buf_.size(),
                                  pbase(), orig_size, NULL);
  // TODO: Should we flush??
  sink_.write(&dest_buf_.front(), comp_size);
}

void LZ4InputBuffer::writeHeader()
{
  size_t ret = LZ4F_compressBegin(ctx_, &dest_buf_.front(), dest_buf_.size(), NULL);
  if (LZ4F_isError(ret))
  {
    printf("Failed to start compression: error %zu", ret);
  }
  sink_.write(&dest_buf_.front(), ret);
}

void LZ4InputBuffer::writeFooter()
{
  size_t ret = LZ4F_compressEnd(ctx_, &dest_buf_.front(), dest_buf_.size(), NULL);
  if (LZ4F_isError(ret))
  {
    printf("Failed to end compression: error %zu", ret);
  }
  sink_.write(&dest_buf_.front(), ret);
}

class LZ4Ostream : public std::ostream
{
 public:
  LZ4Ostream(std::ostream& sink)
    : buffer_(std::make_unique<LZ4InputBuffer>(sink))
  {
    rdbuf(buffer_.get());
  }
 private:
  std::unique_ptr<LZ4InputBuffer> buffer_;
};

int main(int argc, char* argv[])
{
  std::ofstream file("foo.txt");
  LZ4Ostream stream(file);
  std::ifstream infile(argv[1]);
  stream << "Foo me once again" << std::endl;
  std::string line;
  while (getline(infile, line))
  {
    stream << line << '\n';
  }
  stream << "And more" << std::endl;
  return 0;
}
