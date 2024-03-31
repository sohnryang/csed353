#include "tcp_sender.hh"

#include "tcp_config.hh"
#include "tcp_segment.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

RetransmissionTimer::RetransmissionTimer(const uint16_t timeout) : _timeout(timeout) {}

void RetransmissionTimer::tick(const size_t ms_since_last_tick) {
    if (!_is_running)
        return;
    _elapsed += ms_since_last_tick;
}

bool RetransmissionTimer::is_expired() const { return _elapsed >= _timeout; }

void RetransmissionTimer::start() { _is_running = true; }

void RetransmissionTimer::stop() {
    _elapsed = 0;
    _is_running = false;
}

void TCPSender::push_segment(const TCPSegment &segment) {
    _segments_out.push(segment);

    const auto segment_len = segment.length_in_sequence_space();
    if (segment_len > 0) {
        _outstanding_segments.push_back({_next_seqno, segment});
        _next_seqno += segment.length_in_sequence_space();
        _timer.start();
    }
}

vector<TCPSegment> TCPSender::split_to_fit(const TCPSegment &segment) {
    const auto normalized_window_size = max(_window_size, uint16_t{1});

    // TODO: replace with proper implementation
    if (segment.length_in_sequence_space() > normalized_window_size)
        throw runtime_error("Not implemented yet");
    return {segment};
}

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _outstanding_segments()
    , _timer(_initial_retransmission_timeout) {}

size_t TCPSender::bytes_in_flight() const {
    return accumulate(_outstanding_segments.begin(), _outstanding_segments.end(), 0, [](size_t sum, const auto &p) {
        return sum + p.second.length_in_sequence_space();
    });
}

void TCPSender::fill_window() {
    while (true) {
        if (_stream.eof() && _fin_sent)
            return;

        const auto syn = _next_seqno == 0;
        const auto normalized_window_size = static_cast<size_t>(max(_window_size, uint16_t{1}));
        if (normalized_window_size <= bytes_in_flight())
            return;

        const auto available_window_size = normalized_window_size - bytes_in_flight();
        if (available_window_size == 0)
            return;

        TCPSegment segment;
        segment.header().seqno = _isn + _next_seqno;
        segment.header().syn = syn;
        if (segment.length_in_sequence_space() == available_window_size) {
            push_segment(segment);
            return;
        }

        const auto read_size = min({_stream.buffer_size(),
                                    available_window_size - segment.length_in_sequence_space(),
                                    TCPConfig::MAX_PAYLOAD_SIZE});
        segment.payload() = _stream.read(read_size);

        segment.header().fin = _stream.eof() && available_window_size > segment.length_in_sequence_space();
        _fin_sent = segment.header().fin;

        if (segment.length_in_sequence_space() > 0)
            push_segment(segment);
        else
            return;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    const auto absolute_ackno = unwrap(ackno, _isn, _checkpoint);
    if (absolute_ackno > _next_seqno)
        return;

    std::list<std::pair<uint64_t, TCPSegment>> new_outstanding;
    std::copy_if(
        _outstanding_segments.cbegin(),
        _outstanding_segments.cend(),
        std::back_inserter(new_outstanding),
        [absolute_ackno](const auto &p) { return p.first + p.second.length_in_sequence_space() > absolute_ackno; });
    _outstanding_segments = std::move(new_outstanding);

    if (_outstanding_segments.empty())
        _timer.stop();

    if (absolute_ackno > _checkpoint) {
        _timer.timeout() = _initial_retransmission_timeout;
        if (!_outstanding_segments.empty()) {
            _timer.stop();
            _timer.start();
        }
        _retransmission_count = 0;
    }

    _window_size = window_size;
    _checkpoint = absolute_ackno;
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _timer.tick(ms_since_last_tick);
    if (_timer.is_expired()) {
        if (_outstanding_segments.empty())
            throw logic_error("Expected at least one outstanding segment");

        const auto it = min_element(_outstanding_segments.cbegin(),
                                    _outstanding_segments.cend(),
                                    [](const auto &p1, const auto &p2) { return p1.first < p2.first; });

        const auto splitted_segments = split_to_fit(it->second);

        for (auto &segment : splitted_segments)
            _segments_out.emplace(std::move(segment));

        if (_window_size != 0) {
            _timer.timeout() *= 2;
            _retransmission_count++;
        }

        _timer.stop();
        _timer.start();
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retransmission_count; }

void TCPSender::send_empty_segment() {
    TCPSegment segment;
    segment.header().seqno = _isn + _next_seqno;
    push_segment(segment);
}
