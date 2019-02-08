#ifndef LZ4_STREAM
#define LZ4_STREAM

// LZ4 Headers
#include <lz4frame.h>

// Standard headers
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <streambuf>
#include <vector>

namespace lz4_stream {
/**
 * @brief An output stream that will LZ4 compress the input data.
 *
 * An output stream that will wrap another output stream and LZ4
 * compress its input data to that stream.
 *
 */
template <size_t SrcBufSize = 256>
class basic_ostream : public std::ostream
{
 public:
  /**
   * @brief Constructs an LZ4 compression output stream
   *
   * @param sink The stream to write compressed data to
   */
  basic_ostream(std::ostream& sink)
    : std::ostream(new output_buffer(sink)),
      buffer_(dynamic_cast<output_buffer*>(rdbuf())) {
    assert(buffer_);
  }

  /**
   * @brief Destroys the LZ4 output stream. Calls close() if not already called.
   */
  ~basic_ostream() {
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
  class output_buffer : public std::streambuf {
  public:
    output_buffer(const output_buffer &) = delete;
    output_buffer& operator= (const output_buffer &) = delete;

    output_buffer(std::ostream &sink)
      : sink_(sink),
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
    }

    ~output_buffer() {
      close();
    }

    void close() {
      if (closed_) {
        return;
      }
      sync();
      write_footer();
      LZ4F_freeCompressionContext(ctx_);
      closed_ = true;
    }

  private:
    int_type overflow(int_type ch) override {
      assert(std::less_equal<char*>()(pptr(), epptr()));

      *pptr() = static_cast<basic_ostream::char_type>(ch);
      pbump(1);

      if (!header_written_) {
        write_header();
        header_written_ = true;
      }
      compress_and_write();

      return ch;
    }

    int_type sync() override {
      compress_and_write();
      return 0;
    }

    void compress_and_write() {
      // TODO: Throw exception instead or set badbit
      assert(!closed_);
      int orig_size = static_cast<int>(pptr() - pbase());
      pbump(-orig_size);
      size_t comp_size = LZ4F_compressUpdate(ctx_, &dest_buf_.front(), dest_buf_.size(),
                                             pbase(), orig_size, nullptr);
      sink_.write(&dest_buf_.front(), comp_size);
    }

    void write_header() {
      // TODO: Throw exception instead or set badbit
      assert(!closed_);
      size_t ret = LZ4F_compressBegin(ctx_, &dest_buf_.front(), dest_buf_.size(), nullptr);
      if (LZ4F_isError(ret) != 0) {
        throw std::runtime_error(std::string("Failed to start LZ4 compression: ")
                                 + LZ4F_getErrorName(ret));
      }
      sink_.write(&dest_buf_.front(), ret);
    }

    void write_footer() {
      assert(!closed_);
      size_t ret = LZ4F_compressEnd(ctx_, &dest_buf_.front(), dest_buf_.size(), nullptr);
      if (LZ4F_isError(ret) != 0) {
        throw std::runtime_error(std::string("Failed to end LZ4 compression: ")
                                 + LZ4F_getErrorName(ret));
      }
      sink_.write(&dest_buf_.front(), ret);
    }

    std::ostream& sink_;
    std::array<char, SrcBufSize> src_buf_;
    std::vector<char> dest_buf_;
    LZ4F_compressionContext_t ctx_;
    bool closed_;
    bool header_written_ = false;
  };

  output_buffer* buffer_;
};

/**
 * @brief An input stream that will LZ4 decompress output data.
 *
 * An input stream that will wrap another input stream and LZ4
 * decompress its output data to that stream.
 *
 */
template <size_t SrcBufSize = 64 * 1024, size_t DestBufSize = 64 * 1024>
class basic_istream : public std::istream
{
 public:
  /**
   * @brief Constructs an LZ4 decompression input stream
   *
   * @param source The stream to read LZ4 compressed data from
   */
  basic_istream(std::istream& source)
    : std::istream(new input_buffer(source)),
      buffer_(dynamic_cast<input_buffer*>(rdbuf())) {
    assert(buffer_);
  }

  /**
   * @brief Destroys the LZ4 output stream.
   */
  ~basic_istream() {
    delete buffer_;
  }

 private:
  class input_buffer : public std::streambuf {
  public:
    input_buffer(std::istream &source)
      : source_(source),
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

    ~input_buffer() {
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

    input_buffer(const input_buffer&) = delete;
    input_buffer& operator= (const input_buffer&) = delete;
  private:
    std::istream& source_;
    std::array<char, SrcBufSize> src_buf_;
    std::array<char, DestBufSize> dest_buf_;
    size_t offset_;
    size_t src_buf_size_;
    LZ4F_decompressionContext_t ctx_;
  };

  input_buffer* buffer_;
};

using ostream = basic_ostream<>;
using istream = basic_istream<>;

}
#endif // LZ4_STREAM
