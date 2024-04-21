#include "tcp_connection.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"

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

void TCPConnection::send_rst() {
    _sender.segments_out() = {};
    _sender.send_empty_segment();
    _sender.segments_out().front().header().rst = true;
    send_segment();
}

void TCPConnection::kill_connection() {
    _killed = true;
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
}

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _connection_age - _last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    if (!_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0 && !seg.header().syn)
        return;
    if (seg.header().rst) {
        if (!_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0)
            return;
        kill_connection();
    }
    if (seg.length_in_sequence_space() > 0)
        _receiver.segment_received(seg);
    _last_segment_received = _connection_age;
    if (_receiver.stream_out().eof() && !_sender.stream_in().eof())
        _linger_after_streams_finish = false;
    if (seg.header().ack)
        _sender.ack_received(seg.header().ackno, seg.header().win);

    if (_sender.next_seqno_absolute() == 0) {
        _sender.fill_window();
        send_segment();
        return;
    } else if (seg.length_in_sequence_space() > 0 ||
               (_receiver.ackno().has_value() && seg.header().seqno == _receiver.ackno().value() - 1)) {
        _sender.send_empty_segment();
        send_segment();
        return;
    } else if (_sender.bytes_in_flight() == 0) {
        _sender.fill_window();
        send_all_segments();
    }
}

bool TCPConnection::active() const {
    if (_killed)
        return false;
    if (!(_receiver.stream_out().eof() && _sender.stream_in().eof() && _sender.bytes_in_flight() == 0))
        return true;
    if (!_linger_after_streams_finish)
        return false;
    return _connection_age - _last_segment_received < 10 * _cfg.rt_timeout;
}

size_t TCPConnection::write(const string &data) {
    const auto written = _sender.stream_in().write(data);
    _sender.fill_window();
    send_all_segments();
    return written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _connection_age += ms_since_last_tick;
    _sender.tick(ms_since_last_tick);

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        send_rst();
        kill_connection();
        return;
    }

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

            send_rst();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
