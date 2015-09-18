#include <streambuf>
#include <ostream>
#include <vector>
#include <memory>

#include "lz4frame.h"

class LZ4OutputStream : public std::ostream
{
 public:
  LZ4OutputStream(std::ostream& sink)
    : buffer_(std::make_unique<LZ4OuputBuffer>(sink))
  {
    rdbuf(buffer_.get());
  }

 private:
  class LZ4OuputBuffer : public std::streambuf
  {
  public:
    LZ4OuputBuffer(std::ostream &sink);
    ~LZ4OuputBuffer();

    LZ4OuputBuffer(const LZ4OuputBuffer &) = delete;
    LZ4OuputBuffer& operator= (const LZ4OuputBuffer &) = delete;

  private:
    int_type overflow(int_type ch) override;
    int_type sync() override;

    void compressAndWrite();
    void writeHeader();
    void writeFooter();

    std::ostream& sink_;
    std::array<char, 256> src_buf_;
    std::vector<char> dest_buf_;
    LZ4F_compressionContext_t ctx_;
  };

  std::unique_ptr<LZ4OuputBuffer> buffer_;
};
