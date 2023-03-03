#pragma once

#include <fstream>
#include <istream>
#include <ostream>

namespace bonk {

class dev0_buffer : public std::streambuf {
    std::streamsize xsputn(const char* s, std::streamsize n) override {
        return n;
    }
    int overflow(int c) override {
        return c;
    }
};

class dev0_stream : public std::ostream {
    static dev0_buffer nirwana;
  public:
    dev0_stream() : std::ostream(&nirwana) {
    }
};

class InputStream {
  public:
    virtual ~InputStream() = default;

    virtual std::istream& get_stream() const = 0;
};

class StdInputStream : public InputStream {
  public:
    std::istream& stream;

    explicit StdInputStream(std::istream& file) : stream(file) {
    }

    std::istream& get_stream() const override {
        return stream;
    }
};

class FileInputStream : public InputStream {
  public:
    mutable std::ifstream stream;

    explicit FileInputStream(std::string_view filename) : stream(filename) {
    }

    std::istream& get_stream() const override {
        return stream;
    }
};

class OutputStream {
  public:
    virtual ~OutputStream() = default;

    virtual std::ostream& get_stream() const = 0;

    virtual operator bool() const {
        return true;
    }
};

class StdOutputStream : public OutputStream {
  public:
    std::ostream& stream;

    explicit StdOutputStream(std::ostream& file) : stream(file) {
    }

    std::ostream& get_stream() const override {
        return stream;
    }
};

class FileOutputStream : public OutputStream {
  public:
    mutable std::ofstream stream;

    explicit FileOutputStream(std::string_view filename) : stream(filename) {
    }

    std::ostream& get_stream() const override {
        return stream;
    }
};

class NullOutputStream : public OutputStream {
  public:
    static NullOutputStream instance;
    mutable dev0_stream stream;

    std::ostream& get_stream() const override {
        return stream;
    }

    operator bool() const override {
        return false;
    }
};

} // namespace bonk