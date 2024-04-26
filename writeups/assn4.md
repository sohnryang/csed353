Assignment 4 Writeup
=============

My name: 손량

My POVIS ID: ryangsohn

My student ID (numeric): 20220323

This assignment took me about 8 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): none

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): 1.71 Gbit/s, 1.72 Gbit/s

Program Structure and Design of the TCPConnection:

In this assignment, I implemented `TCPConnection` class, which is a wrapper
for `TCPSender` and `TCPReceiver` classes we have implemented in earlier
assignments.

The public member functions, such as `connect`, `write`,
`remaining_outbound_capacity`, `end_input_stream`, `inbound_stream`,
`bytes_in_flight`, etc. are implemented according to the specifications.
Private member functions, `send_segment`, `send_all_segments`, `send_rst`, and
`kill_connection` are implemented for easier segment sending and connection
resetting.

`send_segment` pops a segment from sender's queue and sends it with appropriate
acknowledge number set.

`send_all_segments` pops all segments from sender's queue by repeatedly calling
`send_segment`.

`send_rst` empties the sender's queue, forces the sender to create empty
segment, and send the created segment with RST flag set.

Implementation Challenges:

I struggled for a considerable amount of time to make sure that `TCPConnection`
uses correct acknowledgement number for outbound segments, especially when
communicating with Linux's native TCP implementation.

I also spent some time optimizing the implementation, with some sleepless
nights with flamegraphs and profiler. Interestingly, those benchmark numbers
I have obtained are only reproduced in ARM platforms such as UTM running
on Apple Silicon Macs. For Intel x86 machines the performance was severely
degraded. (about 0.5 Gbit/s) It seems like modulo operations in a tight loop
is detrimental to performance. By replacing modulo operations to conditional
branches, the performance was improved. (about 1.2 Gbit/s)

Remaining Bugs:

None to my knowledge.
