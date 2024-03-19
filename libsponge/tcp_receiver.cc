#include "tcp_receiver.hh"

#include "wrapping_integers.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    if (!_isn.has_value()) {
        if (seg.header().syn)
            _isn = seg.header().seqno;
        else
            return;
    }

    const auto stream_index =
        unwrap(seg.header().seqno, _isn.value(), _reassembler.stream_out().bytes_written()) - !seg.header().syn;
    _reassembler.push_substring(seg.payload().copy(), stream_index, seg.header().fin);
    if (seg.header().fin)
        _fin_received = true;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (_isn.has_value())
        return _isn.value() + _reassembler.stream_out().bytes_written() + 1 +
               static_cast<uint32_t>(_reassembler.empty() && _fin_received);
    else
        return {};
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
