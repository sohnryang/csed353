Assignment 1 Writeup
=============

My name: 손량

My POVIS ID: ryangsohn

My student ID (numeric): 20220323

This assignment took me about 3 hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
I designed `StreamReassembler` based on a ring buffer as I did on assignment
#0. The stream reassembler works by maniuplating `_buffer`, with two pointers:
`_unassembled_start` and `_cur`. The `_cur` pointer points to the index of
first unassembled byte saved in `_buffer`. Thus, we can simply check
`_buffer[_cur]` to determine if there are block of assembled bytes ready to be
pushed to output `ByteStream`. If such block is identified,
`_unassembled_start`, which is the index of first unassembled byte in the
stream, is advanced to the end of the block. Also, `_cur` is advanced
accordingly, marking the place formerly held by the assembled bytes as empty.
This ring buffer approach is efficient as index calculations and pushing a
character to the buffer are done in constant time.

Implementation Challenges:
Although the approach used in this assignment is efficient, it is somewhat
unintuitive to implement as this involves many index translation between index
of byte in the stream vs. index of byte in `_buffer`.

Remaining Bugs: none to my knowledge, as long as the invariant holds (i.e., the
overlapping strings are consistent, etc.)


- Optional: I was surprised by: test library that is very intuitive to use

- Optional: I'm not sure about:
the performace characteristic of ring buffer approached used in my
implementation
