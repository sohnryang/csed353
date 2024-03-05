#include "stream_reassembler.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _unassembled_start(0)
    , _cur(0)
    , _buffer_used(0)
    , _buffer(capacity, -1)
    , _is_finalizing(false)
    , _stream_length(0) {}

size_t StreamReassembler::available_capacity() const { return _output.remaining_capacity(); }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    const size_t sub_stream_end = static_cast<size_t>(index + data.length());
    if (sub_stream_end < _unassembled_start)
        return;
    if (sub_stream_end - _unassembled_start > available_capacity())
        return;

    for (size_t i = 0; i < data.length(); i++) {
        const size_t stream_index = index + i;
        if (stream_index < _unassembled_start)
            continue;

        const size_t unassembled_index = stream_index - _unassembled_start;
        const size_t buffer_index = (unassembled_index + _cur) % _capacity;
        if (_buffer[buffer_index] != -1)
            continue;

        _buffer[buffer_index] = data[i];
    }
    _buffer_used = std::max(sub_stream_end, _buffer_used);

    if (_buffer[_cur] != -1) {
        std::string assembled_prefix;
        for (size_t i = 0; i < _buffer_used; i++) {
            const int ch = _buffer[(_cur + i) % _capacity];
            if (ch == -1)
                break;
            assembled_prefix += static_cast<char>(ch);
        }
        _unassembled_start += assembled_prefix.length();
        _cur = (_cur + assembled_prefix.length()) % _capacity;
        _buffer_used -= assembled_prefix.length();
        _output.write(assembled_prefix);
    }

    if (eof) {
        _is_finalizing = true;
        _stream_length = sub_stream_end;
    }

    if (_is_finalizing && _stream_length == _unassembled_start)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t result = 0;
    for (size_t i = 0; i < _buffer_used; i++) {
        const int ch = _buffer[(_cur + i) % _capacity];
        if (ch != -1)
            result++;
    }
    return result;
}

bool StreamReassembler::empty() const { return _buffer_used == 0; }
