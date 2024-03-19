#include "wrapping_integers.hh"

#include <cstdint>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) { return isn + n; }

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    const auto delta = static_cast<uint32_t>(n - isn);
    const auto checkpoint_lo = checkpoint & 0xffff'ffff, checkpoint_hi = checkpoint - checkpoint_lo;
    if (delta < checkpoint_lo) {
        if (checkpoint_lo - delta < (1ull << 31))
            return checkpoint_hi + delta;
        else
            return checkpoint_hi + (1ull << 32) + delta;
    } else {
        if (delta - checkpoint_lo < (1ull << 31) || checkpoint_hi == 0)
            return checkpoint_hi + delta;
        else
            return checkpoint_hi + delta - (1ull << 32);
    }
}
