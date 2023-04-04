#pragma once

#include <memory>
#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>

namespace bonk {

class dev0_buffer : public std::streambuf {
    std::streamsize xsputn(const char* s, std::streamsize n) override;
    int overflow(int c) override;
};

class dev0_stream : public std::ostream {
    static dev0_buffer nirwana;

  public:
    dev0_stream();
};

class InputStream {
  public:
    virtual ~InputStream() = default;
    virtual std::istream& get_stream() const = 0;
};

struct OneShotReadBuf : public std::streambuf {
    OneShotReadBuf(const char* s, std::size_t n);

  protected:
    std::streambuf::pos_type seekoff(std::streambuf::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
};

class BufferInputStream : public InputStream {
  public:
    std::unique_ptr<OneShotReadBuf> read_buf;
    std::string_view input;
    mutable std::istream stream;

    explicit BufferInputStream(std::string_view input);

    std::istream& get_stream() const override;
    int tell() const;
};

class StdInputStream : public InputStream {
  public:
    std::istream& stream;

    explicit StdInputStream(std::istream& file);
    std::istream& get_stream() const override;
};

class FileInputStream : public InputStream {
  public:
    mutable std::ifstream stream;

    explicit FileInputStream(std::string filename);
    std::istream& get_stream() const override;
};

class OutputStream {
  public:
    virtual ~OutputStream() = default;

    virtual std::ostream& get_stream() const = 0;
    virtual operator bool() const;
};

class StdOutputStream : public OutputStream {
  public:
    std::ostream& stream;

    explicit StdOutputStream(std::ostream& file);
    std::ostream& get_stream() const override;
};

class FileOutputStream : public OutputStream {
  public:
    mutable std::ofstream stream;

    explicit FileOutputStream(std::string filename);
    std::ostream& get_stream() const override;
};

class NullOutputStream : public OutputStream {
  public:
    static NullOutputStream instance;
    mutable dev0_stream stream;

    std::ostream& get_stream() const override;
    operator bool() const override;
};

} // namespace bonk