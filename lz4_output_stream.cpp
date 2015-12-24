// Own headers
#include "lz4_stream.h"

// Standard headers
#include <cassert>
#include <exception>

LZ4OutputStream::LZ4OuputBuffer::LZ4OuputBuffer(std::ostream &sink)
  : sink_(sink),
    dest_buf_(LZ4F_compressBound(src_buf_.size(), NULL))
{
  char* base = &src_buf_.front();
  setp(base, base + src_buf_.size() - 1);

  size_t ret = LZ4F_createCompressionContext(&ctx_, LZ4F_VERSION);
  if (LZ4F_isError(ret))
  {
    throw std::runtime_error(std::string("Failed to create LZ4 compression context: ")
                             + LZ4F_getErrorName(ret));
  }
  writeHeader();
}

LZ4OutputStream::LZ4OuputBuffer::~LZ4OuputBuffer()
{
  sync();
  writeFooter();
  LZ4F_freeCompressionContext(ctx_);
}

LZ4OutputStream::int_type LZ4OutputStream::LZ4OuputBuffer::overflow(int_type ch)
{
  assert(std::less_equal<char*>()(pptr(), epptr()));

  *pptr() = ch;
  pbump(1);

  compressAndWrite();

  return ch;
}

LZ4OutputStream::int_type LZ4OutputStream::LZ4OuputBuffer::sync()
{
  compressAndWrite();
  return 0;
}

void LZ4OutputStream::LZ4OuputBuffer::compressAndWrite()
{
  std::ptrdiff_t orig_size = pptr() - pbase();
  pbump(-orig_size);
  size_t comp_size = LZ4F_compressUpdate(ctx_, &dest_buf_.front(), dest_buf_.size(),
                                         pbase(), orig_size, NULL);
  sink_.write(&dest_buf_.front(), comp_size);
}

void LZ4OutputStream::LZ4OuputBuffer::writeHeader()
{
  size_t ret = LZ4F_compressBegin(ctx_, &dest_buf_.front(), dest_buf_.size(), NULL);
  if (LZ4F_isError(ret))
  {
    throw std::runtime_error(std::string("Failed to start LZ4 compression: ")
                             + LZ4F_getErrorName(ret));
  }
  sink_.write(&dest_buf_.front(), ret);
}

void LZ4OutputStream::LZ4OuputBuffer::writeFooter()
{
  size_t ret = LZ4F_compressEnd(ctx_, &dest_buf_.front(), dest_buf_.size(), NULL);
  if (LZ4F_isError(ret))
  {
    throw std::runtime_error(std::string("Failed to end LZ4 compression: ")
                             + LZ4F_getErrorName(ret));
  }
  sink_.write(&dest_buf_.front(), ret);
}
