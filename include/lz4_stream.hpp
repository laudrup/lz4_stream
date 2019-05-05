#pragma once

// LZ4 Headers
#include <lz4frame.hpp>

// Standard headers
#include <array>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <streambuf>
#include <vector>

namespace metrics
{
std::size_t constexpr BYTE = 8;

std::size_t constexpr KBYTE = BYTE * 1024;

std::size_t constexpr MEGABYTE = KBYTE * 1024;
}

namespace lz4_stream
{
template<std::size_t SrcBufSize = 16 * metrics::MEGABYTE>
class basic_ostream: public std::ostream
{
public:
    explicit basic_ostream(std::ostream & ioSink)
        : std::ostream{new output_buffer{ioSink}}
        , m_buffer{reinterpret_cast<output_buffer *>(rdbuf())}
    {}

    ~basic_ostream() final
    {
        this->Close();
        delete m_buffer;
    }

    void Close()
    {
        m_buffer->Close();
    }

private:
    class output_buffer: public std::streambuf
    {
    public:
        output_buffer(output_buffer const & iOther) = delete;
        output_buffer & operator=(output_buffer const & iOther) = delete;

        explicit output_buffer(std::ostream & ioSink)
            : m_sink{ioSink}
            , m_context{nullptr}
            , m_closed{false}
        {
            std::size_t && destinationBufferSize = LZ4F_compressBound(SrcBufSize, nullptr);
            m_destinationBuffer.resize(destinationBufferSize);

            char * base = m_sourceBuffer.data();
            setp(base, base + m_sourceBuffer.size() - 1);

            std::size_t const result = LZ4F_createCompressionContext(&m_context, LZ4F_VERSION);
            if(LZ4F_isError(result) not_eq 0) {
                std::string error;
                error += "Failed to create LZ4 compression context: ";
                error += LZ4F_getErrorName(result);
                throw std::runtime_error{error};
            }
            this->WriteHeader();
        }

        ~output_buffer() final
        {
            this->Close();
        }

        void Close()
        {
            if(m_closed)
                return;

            this->sync();
            this->WriteFooter();
            LZ4F_freeCompressionContext(m_context);
            m_closed = true;
        }

    private:
        int_type overflow(int_type iCharacter) final
        {
            *pptr() = static_cast<basic_ostream::char_type>(iCharacter);
            pbump(1);
            this->CompressAndWrite();

            return iCharacter;
        }

        int_type sync() final
        {
            this->CompressAndWrite();
            return 0;
        }

        void CompressAndWrite()
        {
            auto const orig_size = static_cast<std::int32_t>(pptr() - pbase());
            pbump(-orig_size);
            std::size_t const result = LZ4F_compressUpdate(
                m_context,
                m_destinationBuffer.data(),
                m_destinationBuffer.capacity(),
                pbase(),
                orig_size,
                nullptr
            );

            if(LZ4F_isError(result) not_eq 0) {
                std::string error;
                error += "LZ4 compression failed: ";
                error += LZ4F_getErrorName(result);
                throw std::runtime_error{error};
            }

            m_sink.write(m_destinationBuffer.data(), result);
        }

        void WriteHeader()
        {
            std::size_t const result = LZ4F_compressBegin(
                m_context,
                m_destinationBuffer.data(),
                m_destinationBuffer.capacity(),
                nullptr
            );

            if(LZ4F_isError(result) not_eq 0) {
                std::string error;
                error += "Failed to start LZ4 compression: ";
                error += LZ4F_getErrorName(result);
                throw std::runtime_error{error};
            }
            m_sink.write(m_destinationBuffer.data(), result);
        }

        void WriteFooter()
        {
            assert(!m_closed);
            std::size_t const result = LZ4F_compressEnd(
                m_context,
                m_destinationBuffer.data(),
                m_destinationBuffer.capacity(),
                nullptr
            );
            if(LZ4F_isError(result) not_eq 0) {
                std::string error;
                error += "Failed to end LZ4 compression: ";
                error += LZ4F_getErrorName(result);
                throw std::runtime_error{error};
            }
            m_sink.write(m_destinationBuffer.data(), result);
        }

        std::ostream & m_sink;
        std::array<char, SrcBufSize> m_sourceBuffer;
        std::vector<char> m_destinationBuffer;
        LZ4F_compressionContext_t m_context;
        bool m_closed;
    };

    output_buffer * m_buffer;
};

template<std::size_t SrcBufSize = 16 * metrics::MEGABYTE, std::size_t DestBufSize = 16 * metrics::MEGABYTE>
class basic_istream: public std::istream
{
public:
    explicit basic_istream(std::istream & iSource)
        : std::istream{new input_buffer(iSource)}, m_buffer{static_cast<input_buffer *>(rdbuf())}
    {}

    ~basic_istream() final
    {
        delete m_buffer;
    }

private:
    class input_buffer: public std::streambuf
    {
    public:
        input_buffer(input_buffer const & iOther) = delete;
        input_buffer & operator=(input_buffer const & iOther) = delete;

        explicit input_buffer(std::istream & iSource)
            : m_source{iSource}
            , m_offset{0}
            , m_sourceBufferSize{0}
            , m_context{nullptr}
        {
            std::size_t const result = LZ4F_createDecompressionContext(&m_context, LZ4F_VERSION);
            if(LZ4F_isError(result) not_eq 0) {
                std::string error;
                error += "Failed to create LZ4 decompression context: ";
                error += LZ4F_getErrorName(result);
                throw std::runtime_error{error};
            }
            setg(sourceBuffer.data(), sourceBuffer.data(), sourceBuffer.data());
        }

        ~input_buffer() final
        {
            LZ4F_freeDecompressionContext(m_context);
        }

        int_type underflow() final
        {
            std::size_t writtenSize = 0;
            while (writtenSize == 0) {
                if(m_offset == m_sourceBufferSize) {
                    m_source.read(sourceBuffer.data(), sourceBuffer.size());
                    m_sourceBufferSize = static_cast<std::size_t>(m_source.gcount());
                    m_offset = 0;
                }

                if(m_sourceBufferSize == 0) {
                    return traits_type::eof();
                }

                std::size_t src_size = m_sourceBufferSize - m_offset;
                std::size_t dest_size = destinationBuffer.size();
                std::size_t const result = LZ4F_decompress(
                    m_context,
                    destinationBuffer.data(),
                    &dest_size,
                    sourceBuffer.data() + m_offset,
                    &src_size,
                    nullptr
                );

                if(LZ4F_isError(result) != 0) {
                    std::string error;
                    error += "LZ4 decompression failed: ";
                    error += LZ4F_getErrorName(result);
                    throw std::runtime_error{error};
                }
                writtenSize = dest_size;
                m_offset += src_size;
            }

            setg(destinationBuffer.data(), destinationBuffer.data(), destinationBuffer.data() + writtenSize);
            return traits_type::to_int_type(*gptr());
        }

    private:
        std::istream & m_source;
        std::array<char, SrcBufSize> sourceBuffer;
        std::array<char, DestBufSize> destinationBuffer;
        std::size_t m_offset;
        std::size_t m_sourceBufferSize;
        LZ4F_decompressionContext_t m_context;
    };

    input_buffer * m_buffer;
};

using ostream = basic_ostream<>;
using istream = basic_istream<>;

}
