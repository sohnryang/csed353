Assignment 3 Writeup
=============

My name: 손량

My POVIS ID: ryangsohn

My student ID (numeric): 20220323

This assignment took me about 4 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): none

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:

## Overall Design of `TCPSender`

`TCPSender` manages in-flight segments in a simple `std::list` of `( absolute
sequence number, TCPSegment)` pair. The member functions of `TCPSender` such as
`TCPSender::fill_window`, `TCPSender::ack_received` and `TCPSender::tick` use
this list to access in-flight segments. `TCPSender` uses additional `_fin_sent`
field to remember whether FIN packet was sent, and `_checkpoint` field to unwrap
sequence numbers in `WrappingInt32` type. Also, code related to retransmission
timer belongs to a separate class `RetransmissionTimer` for better organization.

### Implementation of `fill_window`

Before pushing any segments to the `_segments_out` queue, the function first
checks if the stream is finalized by sending FIN packet. If so, there is no
point of filling the window and the function simply quits.

If otherwise, by checking the value of `_next_seqno`, SYN flag is determined,
payload is read from the stream and FIN is set if needed. The prepared segment
is sent only when its length (in sequence space) is nonzero. If segment with
zero length is generated, it indicates that the sender has to wait for more data
or EOF signal from input `ByteStream` so the loop quits. Segments are pushed
using `push_segment`, which handles increment of `_next_seqno`, management of
outstanding segments and start/stop of retransmission timer.

### Implementation of `ack_received`

When an ACK packet is received, the sequence number is checked first to ensure
that the ACK is for already sent segment. Afterwards, the list of in-flight
segment is filtered to remove acknowleged segments. If the ACK is not for a
previously acknowleged segment, retransmission timer and counter is handled
according to spec. Finally, the window size and checkpoint is adjusted according
to the values in the segment header.

### Implementation of `tick`

First the retransmission timer tick is updated according to the supplied
argument. If the timer is expired, then the segment with lowest absolute
sequence number is selected using `std::min_element` and retransmitted, updating
timer states as the TCP spec mandates.

## Retransmission Timer Implementation

Retransmission timer is a simple state machine which manages timeout and elapsed
time with `tick`, `start`, `stop`, and `timeout` member functions.

Implementation Challenges:

Other than understanding the TCP spec, there was no significant challenge.

Remaining Bugs: none to my knowledge

- Optional: I had unexpected difficulty with: fixing bugs due to stupid mistakes

- Optional: I'm not sure about: the efficiency of my implementation
