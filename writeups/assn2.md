Assignment 2 Writeup
=============

My name: 손량

My POVIS ID: ryangsohn

My student ID (numeric): 20220323

This assignment took me about 3 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): none

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:

## `WrappingInt32`: Wrapping Integers

`WrappingInt32::wrap` is simply implemented by using + operator overload of
`WrappingInt32` class.

`WrappingInt32::unwrap` works by first calculating the difference between `n`
and `isn`, then determine the final value based on the provided `checkpoint`.
First, the `checkpoint` variable is split into `checkpoint_lo` and
`checkpoint_hi` using bit masking. Then, the difference `n - isn` by `delta`
is calculated and the return value is determined with the comparison result and
comparison between the difference and `1ull << 31`. Since the difference is 32
bit and `checkpoint_hi` is `checkpoint` with zeroed-out lower 32 bits, we can
simply add them with some adjustments.

## `TCPReceiver`: Receiving TCP Streams

`TCPReceiver` is mostly a wrapper around `StreamReassembler`. `TCPReceiver`
keeps the initial sequence number in `TCPReceiver::_isn` and uses this field
when calculating the stream offset of the given byte.

`TCPReceiver::segment_received` checks for SYN flag and sets `_isn` if SYN flag
is set. For segments with SYN flag on, they are the first segment of the stream,
so the stream index of the bytes in the segment is the unwrapped sequence
number. Otherwise, the stream index is unwrapped sequence number minus one, as
the SYN flag takes up one sequence number. Then, the data is pushed to stream
reassembler. If the segment has FIN flag set, `_fin_received` is set to `true`
in order to indicate that the FIN byte is received and should be taken into
account when calculating ACK number.

`TCPReceiver::ackno` calculates the acknowledge number. If `_isn` field is set
with proper value, then the acknowledge number is calculated using the initial
sequence number and bytes written to the stream. If FIN flag is received and
the stream is empty, this means that FIN flag is consumed and the final value is
incremented by one. If `_isn` field is not set, then `std::nullopt` is returned
instead.

Implementation Challenges:
This assignment is building a wrapper around `StreamReassembler`, so there was
no significat difficulty in implementation.

Remaining Bugs:
None to my knowledge.

- Optional: I'm not sure about: The performance of my implementation
