#include "tcp_connection.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPConnection::send_segment() {
    auto &sender_segments = _sender.segments_out();
    if (sender_segments.empty())
        return;

    auto segment = sender_segments.front();
    sender_segments.pop();

    segment.header().win = min(_receiver.window_size(), static_cast<size_t>(numeric_limits<uint16_t>::max()));
    if (_receiver.ackno().has_value()) {
        segment.header().ack = true;
        segment.header().ackno = _receiver.ackno().value();
    } else
        segment.header().ack = false;
    _segments_out.push(segment);
}

void TCPConnection::send_all_segments() {
    while (!_sender.segments_out().empty())
        send_segment();
}

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return {}; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (seg.header().rst) {
        // TODO: handle RST
    }
    _receiver.segment_received(seg);
    if (_receiver.stream_out().eof() && !_sender.stream_in().eof())
        _linger_after_streams_finish = false;
    if (seg.header().ack)
        _sender.ack_received(seg.header().ackno, seg.header().win);

    if (seg.length_in_sequence_space() > 0 ||
        (_receiver.ackno().has_value() && seg.header().seqno == _receiver.ackno().value() - 1))
        _sender.send_empty_segment();
    send_segment();
}

bool TCPConnection::active() const {
    if (!(_receiver.stream_out().eof() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0))
        return true;
    if (!_linger_after_streams_finish)
        return false;
    return true;  // TODO: implement timeout
}

size_t TCPConnection::write(const string &data) {
    const auto written = _sender.stream_in().write(data);
    _sender.fill_window();
    send_all_segments();
    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);
    send_all_segments();
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    send_all_segments();
}

void TCPConnection::connect() {
    _sender.fill_window();
    send_all_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
