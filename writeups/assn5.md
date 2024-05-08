Assignment 5 Writeup
=============

My name: 손량

My POVIS ID: ryangsohn

My student ID (numeric): 20220323

This assignment took me about 3 hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): none

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:

The `NetworkInterface` class sends and receives IPv4 datagrams using Ethernet
frames. Since sending and receiving IPv4 datagrams need MAC address of receiver,
I implemented the class to send ARP requests when needed.

## `NetworkInterface::resolve_ip` -- resolve IP address to Ethernet address

`NetworkInterface` manages IP-to-MAC address mappings in `_known_addresses`.
Also, before the Ethernet address to the corresponding IP address is known,
the frames with yet-to-be resolved destinations are stored in hash table,
`_unresolved_frames`. Resolving IP address to Ethernet address involves adding
an entry to `_known_addresses`, marking the ARP request with corresponding IP
address as done, and sending all pending frames stored in `_unresolved_frames`.
Such functionality is implemented in this function.

## `NetworkInterface::send_datagram` -- send IPv4 datagram

This function first checks if the Ethernet address corresponding to the
destination IP is known. If the Ethernet address is known, the given
IPv4 datagram is wrapped in `EthernetFrame` and sent right away. If it is
not the case, then ARP request is broadcasted in order to resolve the IP
address. The given datagram is saved into `_unresolved_frames`, in a form of
partially-constructed `EthernetFrame` whose destination address is not set. When
sending ARP request, the table of in-flight ARP requests is checked to avoid
congestion.

## `NetworkInterface::recv_frame` -- receive Ethernet frame

Following the spec, this function ignores Ethernet frames with incorrect
recipient address. If the frame contains valid IPv4 datagram, the function
simply returns the parsed IPv4 datagram. If the frame contains valid ARP
message, the function checks whether the message is request or reply. If the
message is ARP request asking for its Ethernet address, ARP reply is sent back
to the sender and sender's IP-MAC pair is resolved. If the message is reply, the
requested IP-MAC pair is also resolved.

## `NetworkInterface::tick` -- register ticks

This function updates the network interface's `_age` variable and update
outdated mappings and in-flight ARP segments.

Implementation Challenges:

There was no significant implementation challenges in this assignment.

Remaining Bugs: None to my knowledge.
