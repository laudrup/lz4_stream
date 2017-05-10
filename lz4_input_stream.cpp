// Own headers
#include "lz4_stream.h"

LZ4InputStream::LZ4InputBuffer::LZ4InputBuffer(std::istream &source)
  : source_(source),
    src_buf_(),
    dest_buf_(),
    offset_(0),
    src_buf_size_(0),
    ctx_(nullptr)
{
  size_t ret = LZ4F_createDecompressionContext(&ctx_, LZ4F_VERSION);
  if (LZ4F_isError(ret) != 0)
  {
    throw std::runtime_error(std::string("Failed to create LZ4 decompression context: ")
                             + LZ4F_getErrorName(ret));
  }
  setg(&src_buf_.front(), &src_buf_.front(), &src_buf_.front());
}

LZ4InputStream::int_type LZ4InputStream::LZ4InputBuffer::underflow()
{
  size_t ret;

  while(true) {
    if (srcPtr == srcEnd) {
      source_.read(&src_buf_.front(), src_buf_.size());
      src_buf_size_ = static_cast<size_t>(source_.gcount());

      if (src_buf_size_ == 0) {
        return traits_type::eof();
      }

      srcPtr = &src_buf_.front();
      srcEnd = srcPtr + src_buf_size_;
    }

    if (!readHeader) {
      LZ4F_frameInfo_t info;
      ret = LZ4F_getFrameInfo(ctx_, &info, srcPtr, &src_buf_size_);
      if (LZ4F_isError(ret)) {
        throw std::runtime_error(std::string("LZ4F_getFrameInfo error: ")
           + LZ4F_getErrorName(ret));
      }

      srcPtr += src_buf_size_;
      src_buf_size_ = srcEnd - srcPtr;
      readHeader = true;
    }

    while (srcPtr != srcEnd) {
       /* INVARIANT: Any data left in dst has already been written */
      size_t dstSize = dest_buf_.size();
      ret = LZ4F_decompress(ctx_, &dest_buf_.front(), &dstSize, srcPtr,
        &src_buf_size_, /* LZ4F_decompressOptions_t */ NULL);

      if (LZ4F_isError(ret)) {
        throw std::runtime_error(std::string("Decompression error: ")
           + LZ4F_getErrorName(ret));
      }

      /* Update input */
      srcPtr += src_buf_size_;
      src_buf_size_ = srcEnd - srcPtr;

      /* Flush output */
      if (dstSize != 0) {
        setg(&dest_buf_.front(), &dest_buf_.front(), &dest_buf_.front() + dstSize);

        return traits_type::to_int_type(*gptr());
      }
    }
  }
}

LZ4InputStream::LZ4InputBuffer::~LZ4InputBuffer()
{
  LZ4F_freeDecompressionContext(ctx_);
}
