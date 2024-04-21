#include "stream_reassembler.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity)
    , _capacity(capacity)
    , _unassembled_start(0)
    , _cur(0)
    , _buffer_used(0)
    , _buffer(capacity, std::nullopt)
    , _is_finalizing(false)
    , _stream_length(0) {}

size_t StreamReassembler::available_capacity() const { return _output.remaining_capacity(); }

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const uint64_t index, const bool eof) {
    const size_t stream_copy_start = max(static_cast<size_t>(index), _unassembled_start),
                 stream_data_end = index + data.length(),
                 stream_copy_end = min(stream_data_end, _unassembled_start + available_capacity());
    for (size_t stream_index = stream_copy_start; stream_index < stream_copy_end; stream_index++) {
        const size_t unassembled_index = stream_index - _unassembled_start;
        const size_t buffer_index = (unassembled_index + _cur) % _capacity;
        if (_buffer[buffer_index].has_value())
            continue;

        _buffer[buffer_index] = data[stream_index - index];
    }
    if (stream_copy_end >= _unassembled_start)
        _buffer_used = max(stream_copy_end - _unassembled_start, _buffer_used);

    if (_buffer[_cur].has_value()) {
        string assembled_prefix;
        assembled_prefix.assign(_buffer_used, 0);
        size_t i;
        for (i = 0; i < _buffer_used; i++) {
            auto &ch = _buffer[(_cur + i) % _capacity];
            if (!ch.has_value())
                break;
            assembled_prefix[i] = ch.value();
            ch.reset();
        }
        assembled_prefix.resize(min(i, _buffer_used));
        _unassembled_start += assembled_prefix.length();
        _cur = (_cur + assembled_prefix.length()) % _capacity;
        _buffer_used -= assembled_prefix.length();
        _output.write(assembled_prefix);
    }

    if (eof && stream_data_end == stream_copy_end) {
        _is_finalizing = true;
        _stream_length = stream_copy_end;
    }

    if (_is_finalizing && _stream_length == _unassembled_start)
        _output.end_input();
}

size_t StreamReassembler::unassembled_bytes() const {
    size_t result = 0;
    for (size_t i = 0; i < _buffer_used; i++) {
        const auto &ch = _buffer[(_cur + i) % _capacity];
        if (ch.has_value())
            result++;
    }
    return result;
}

bool StreamReassembler::empty() const { return _buffer_used == 0; }
