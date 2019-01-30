#ifndef LZ4_STREAM
#define LZ4_STREAM

// LZ4 Headers
#include <lz4frame.h>

// Standard headers
#include <cassert>
#include <streambuf>
#include <iostream>
#include <vector>
#include <memory>
#include <array>

/**
 * @brief An output stream that will LZ4 compress the input data.
 *
 * An output stream that will wrap another output stream and LZ4
 * compress its input data to that stream.
 *
 */
class LZ4OutputStream : public std::ostream
{
 public:
  /**
   * @brief Constructs an LZ4 compression output stream
   *
   * @param sink The stream to write compressed data to
   */
  LZ4OutputStream(std::ostream& sink)
    : std::ostream(new LZ4OutputBuffer(sink)),
      buffer_(dynamic_cast<LZ4OutputBuffer*>(rdbuf())) {
    assert(buffer_);
  }

  /**
   * @brief Destroys the LZ4 output stream. Calls close() if not already called.
   */
  ~LZ4OutputStream() {
    close();
    delete buffer_;
  }

  /**
   * @brief Flushes and writes LZ4 footer data to the LZ4 output stream.
   *
   * After calling this function no more data should be written to the stream.
   */
  void close() {
    buffer_->close();
  }

 private:
  class LZ4OutputBuffer : public std::streambuf {
  public:
    LZ4OutputBuffer(const LZ4OutputBuffer &) = delete;
    LZ4OutputBuffer& operator= (const LZ4OutputBuffer &) = delete;

    LZ4OutputBuffer(std::ostream &sink)
      : sink_(sink),
        src_buf_(),
        // TODO: No need to recalculate the dest_buf_ size on each construction
        dest_buf_(LZ4F_compressBound(src_buf_.size(), nullptr)),
        ctx_(nullptr),
        closed_(false) {
      char* base = &src_buf_.front();
      setp(base, base + src_buf_.size() - 1);

      size_t ret = LZ4F_createCompressionContext(&ctx_, LZ4F_VERSION);
      if (LZ4F_isError(ret) != 0) {
        throw std::runtime_error(std::string("Failed to create LZ4 compression context: ")
                                 + LZ4F_getErrorName(ret));
      }
      writeHeader();
    }

    ~LZ4OutputBuffer() {
      close();
    }

    void close() {
      if (closed_) {
        return;
      }
      sync();
      writeFooter();
      LZ4F_freeCompressionContext(ctx_);
      closed_ = true;
    }

  private:
    int_type overflow(int_type ch) override {
      assert(std::less_equal<char*>()(pptr(), epptr()));

      *pptr() = static_cast<LZ4OutputStream::char_type>(ch);
      pbump(1);

      compressAndWrite();

      return ch;
    }

    int_type sync() override {
      compressAndWrite();
      return 0;
    }

    void compressAndWrite() {
      // TODO: Throw exception instead or set badbit
      assert(!closed_);
      std::ptrdiff_t orig_size = pptr() - pbase();
      pbump(-orig_size);
      size_t comp_size = LZ4F_compressUpdate(ctx_, &dest_buf_.front(), dest_buf_.size(),
                                             pbase(), orig_size, nullptr);
      sink_.write(&dest_buf_.front(), comp_size);
    }

    void writeHeader() {
      // TODO: Throw exception instead or set badbit
      assert(!closed_);
      size_t ret = LZ4F_compressBegin(ctx_, &dest_buf_.front(), dest_buf_.size(), nullptr);
      if (LZ4F_isError(ret) != 0) {
        throw std::runtime_error(std::string("Failed to start LZ4 compression: ")
                                 + LZ4F_getErrorName(ret));
      }
      sink_.write(&dest_buf_.front(), ret);
    }

    void writeFooter() {
      assert(!closed_);
      size_t ret = LZ4F_compressEnd(ctx_, &dest_buf_.front(), dest_buf_.size(), nullptr);
      if (LZ4F_isError(ret) != 0) {
        throw std::runtime_error(std::string("Failed to end LZ4 compression: ")
                                 + LZ4F_getErrorName(ret));
      }
      sink_.write(&dest_buf_.front(), ret);
    }

    std::ostream& sink_;
    std::array<char, 256> src_buf_;
    std::vector<char> dest_buf_;
    LZ4F_compressionContext_t ctx_;
    bool closed_;
  };

  LZ4OutputBuffer* buffer_;
};

/**
 * @brief An input stream that will LZ4 decompress output data.
 *
 * An input stream that will wrap another input stream and LZ4
 * decompress its output data to that stream.
 *
 */
class LZ4InputStream : public std::istream
{
 public:
  /**
   * @brief Constructs an LZ4 decompression input stream
   *
   * @param source The stream to read LZ4 compressed data from
   */
  LZ4InputStream(std::istream& source)
    : std::istream(new LZ4InputBuffer(source)),
      buffer_(dynamic_cast<LZ4InputBuffer*>(rdbuf())) {
    assert(buffer_);
  }

  /**
   * @brief Destroys the LZ4 output stream.
   */
  ~LZ4InputStream() {
    delete buffer_;
  }

 private:
  class LZ4InputBuffer : public std::streambuf {
  public:
    LZ4InputBuffer(std::istream &source)
      : source_(source),
        src_buf_(),
        dest_buf_(),
        offset_(0),
        src_buf_size_(0),
        ctx_(nullptr) {
      size_t ret = LZ4F_createDecompressionContext(&ctx_, LZ4F_VERSION);
      if (LZ4F_isError(ret) != 0) {
        throw std::runtime_error(std::string("Failed to create LZ4 decompression context: ")
                                 + LZ4F_getErrorName(ret));
      }
      setg(&src_buf_.front(), &src_buf_.front(), &src_buf_.front());
    }

    ~LZ4InputBuffer() {
      LZ4F_freeDecompressionContext(ctx_);
    }

    int_type underflow() override {
      while (true) {
        if (offset_ == src_buf_size_) {
          source_.read(&src_buf_.front(), src_buf_.size());
          src_buf_size_ = static_cast<size_t>(source_.gcount());
          offset_ = 0;
        }

        if (src_buf_size_ == 0) {
          return traits_type::eof();
        }

        size_t src_size = src_buf_size_ - offset_;
        size_t dest_size = dest_buf_.size();
        size_t ret = LZ4F_decompress(ctx_, &dest_buf_.front(), &dest_size,
                                     &src_buf_.front() + offset_, &src_size, nullptr);
        offset_ += src_size;
        if (LZ4F_isError(ret) != 0) {
          throw std::runtime_error(std::string("LZ4 decompression failed: ")
                                   + LZ4F_getErrorName(ret));
        }

        if (dest_size != 0) {
          setg(&dest_buf_.front(), &dest_buf_.front(), &dest_buf_.front() + dest_size);
          return traits_type::to_int_type(*gptr());
        }
      }
    }

    LZ4InputBuffer(const LZ4InputBuffer&) = delete;
    LZ4InputBuffer& operator= (const LZ4InputBuffer&) = delete;
  private:
    std::istream& source_;
    std::array<char, 64 * 1024> src_buf_;
    std::array<char, 64 * 1024> dest_buf_;
    size_t offset_;
    size_t src_buf_size_;
    LZ4F_decompressionContext_t ctx_;
  };

  LZ4InputBuffer* buffer_;
};

#endif // LZ4_STREAM
