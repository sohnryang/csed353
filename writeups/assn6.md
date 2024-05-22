Assignment 6 Writeup
=============

My name: 손량

My POVIS ID: ryangsohn

My student ID (numeric): 20220323

This assignment took me about 2 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): none

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:

The `Router` class implements basic O(N) route matching. Routing table is stored
in `_routing_table` array of `RouteEntry` structs, and longest-prefix matching
is done for every entries when datagrams are received.

## `Router::add_route` -- add routing table entry

This function simply constructs `RouteEntry` object and appends to
`_routing_table` array.

## `Router::route_one_datagram` -- route a datagram

When a datagram is received, longest prefix matching is done and datagram is
pushed to matching interface with TTL set according to specs. If TTL is one or
zero, the datagram is discarded.

## `Router::prefix_match_len` -- calculate matching bits of IP address

Given two IP addresses represented in `uint32_t`, calculate the number of
matching prefix bits. Counting matching prefix bit length is done by XORing
two addresses and counting leading zeroes with `__builtin_clz` function. The
XOR result has bits set only on places where the bits of two addresses does not
match. This means that the number of leading zeroes is equal to the length of
matching prefix. Instead of hand-implementing the counting for leading zeroes,
GCC extension function is used.

## `Router::longest_prefix_match` -- get the longest prefix match

This function iterates over the routing table and calculates prefix match
lengths of each entries, and determines the route entry with longest match. If
there are no matches in routing table, the function returns `std::nullopt`.

Implementation Challenges:

There was no significant implementation challenges in this assignment.

Remaining Bugs:

None to my knowledge.
