#include "router.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

uint8_t Router::prefix_match_len(const uint32_t x, const uint32_t y) {
    const auto mismatch_bits = x ^ y;
    return mismatch_bits ? __builtin_clz(mismatch_bits) : 32;
}

optional<Router::RouteEntry> Router::longest_prefix_match(const uint32_t address) {
    const auto max_it = max_element(_routing_table.begin(),
                                    _routing_table.end(),
                                    [this, address](const Router::RouteEntry &e1, const Router::RouteEntry &e2) {
                                        const auto e1_match_len = prefix_match_len(address, e1.route_prefix),
                                                   e2_match_len = prefix_match_len(address, e2.route_prefix);
                                        if (e1_match_len < e1.prefix_length)
                                            return true;
                                        else if (e2_match_len < e2.prefix_length)
                                            return false;
                                        return e1.prefix_length < e2.prefix_length;
                                    });
    if (prefix_match_len(address, max_it->route_prefix) >= max_it->prefix_length)
        return *max_it;
    else
        return {};
}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    cerr << "DEBUG: adding route " << Address::from_ipv4_numeric(route_prefix).ip() << "/" << int(prefix_length)
         << " => " << (next_hop.has_value() ? next_hop->ip() : "(direct)") << " on interface " << interface_num << "\n";

    _routing_table.push_back({route_prefix, prefix_length, next_hop, interface_num});
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    const auto dst_address = dgram.header().dst;
    const auto route_entry = longest_prefix_match(dst_address);
    if (!route_entry.has_value())
        return;

    if (dgram.header().ttl <= 1)
        return;
    dgram.header().ttl--;

    const auto unwrapped = route_entry.value();
    const auto interface_num = unwrapped.interface_num;
    const auto next_hop = unwrapped.next_hop.value_or(Address::from_ipv4_numeric(dst_address));
    interface(interface_num).send_datagram(dgram, next_hop);
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}
