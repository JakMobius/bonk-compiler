
#include <iostream>
#include "streams.hpp"

namespace bonk {

dev0_buffer dev0_stream::nirwana{};
dev0_stream::dev0_stream() : std::ostream(&nirwana) {
}
NullOutputStream NullOutputStream::instance{};
std::ostream& NullOutputStream::get_stream() const {
    return stream;
}
NullOutputStream::operator bool() const {
    return false;
}

std::streamsize dev0_buffer::xsputn(const char* s, std::streamsize n) {
    return n;
}
int dev0_buffer::overflow(int c) {
    return c;
}
OneShotReadBuf::OneShotReadBuf(const char* s, std::size_t n) {
    setg((char*)s, (char*)s, (char*)s + n);
}
std::streambuf::pos_type OneShotReadBuf::seekoff(std::streambuf::off_type off,
                                                 std::ios_base::seekdir dir,
                                                 std::ios_base::openmode which) {
    if (dir == std::ios_base::beg) {
        setg(eback(), eback() + off, egptr());
    } else if (dir == std::ios_base::cur) {
        setg(eback(), gptr() + off, egptr());
    } else if (dir == std::ios_base::end) {
        setg(eback(), egptr() + off, egptr());
    }

    return gptr() - eback();
}
BufferInputStream::BufferInputStream(std::string_view input)
    : input(input), read_buf(std::make_unique<OneShotReadBuf>(input.data(), input.size())),
      stream(read_buf.get()) {
}
std::istream& BufferInputStream::get_stream() const {
    return stream;
}
int BufferInputStream::tell() const {
    return stream.tellg();
}
StdInputStream::StdInputStream(std::istream& file) : stream(file) {
}
std::istream& StdInputStream::get_stream() const {
    return stream;
}
FileInputStream::FileInputStream(std::string filename) : stream(filename) {
}
std::istream& FileInputStream::get_stream() const {
    return stream;
}
OutputStream::operator bool() const {
    return true;
}
StdOutputStream::StdOutputStream(std::ostream& file) : stream(file) {
}
std::ostream& StdOutputStream::get_stream() const {
    return stream;
}
FileOutputStream::FileOutputStream(std::string filename) : stream(filename) {
}
std::ostream& FileOutputStream::get_stream() const {
    return stream;
}
}