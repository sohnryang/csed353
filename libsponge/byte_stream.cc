#include "byte_stream.hh"

#include <algorithm>
#include <stdexcept>
#include <string>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _input_eof(false)
    , _capacity(capacity)
    , _buffer_used(0)
    , _cur(0)
    , _buffer(capacity, -1)
    , _read_bytes(0)
    , _written_bytes(0) {}

size_t ByteStream::write(const std::string &data) {
    const auto write_bytes = std::min(_capacity - _buffer_used, data.length());
    for (size_t i = 0; i < write_bytes; i++)
        _buffer[(_cur + _buffer_used + i) % _capacity] = data[i];
    _buffer_used += write_bytes;
    _written_bytes += write_bytes;
    return write_bytes;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if (len > _buffer_used)
        throw std::length_error("Tried to read more than capacity");
    std::string result;
    for (size_t i = 0; i < len; i++)
        result.push_back(_buffer[(_cur + i) % _capacity]);
    return result;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    if (len > _buffer_used)
        throw std::length_error("Tried to read more than capacity");
    _cur = (_cur + len) % _capacity;
    _buffer_used -= len;
    _read_bytes += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    const std::string result = peek_output(len);
    pop_output(len);
    return result;
}

void ByteStream::end_input() { _input_eof = true; }

bool ByteStream::input_ended() const { return _input_eof; }

size_t ByteStream::buffer_size() const { return _buffer_used; }

bool ByteStream::buffer_empty() const { return _buffer_used == 0; }

bool ByteStream::eof() const { return _input_eof && _buffer_used == 0; }

size_t ByteStream::bytes_written() const { return _written_bytes; }

size_t ByteStream::bytes_read() const { return _read_bytes; }

size_t ByteStream::remaining_capacity() const { return _capacity - _buffer_used; }
