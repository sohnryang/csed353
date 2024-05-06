#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"
#include "ethernet_header.hh"
#include "ipv4_datagram.hh"
#include "parser.hh"

#include <cstddef>
#include <iostream>
#include <limits>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

using namespace std;

void NetworkInterface::resolve_ip(const std::uint32_t ip_address, const EthernetAddress &ethernet_address) {
    _known_addresses[ip_address] = {ethernet_address, _age + ARP_MAPPING_MAXAGE};
    _arp_in_flight_timestamp.erase(ip_address);

    auto &frames = _unresolved_frames[ip_address];
    while (!frames.empty()) {
        auto frame = frames.front();
        frames.pop();

        frame.header().dst = ethernet_address;
        _frames_out.push(frame);
    }
}

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address)
    , _ip_address(ip_address)
    , _known_addresses{{ip_address.ipv4_numeric(), {ethernet_address, numeric_limits<size_t>::max()}}} {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    EthernetFrame frame;
    frame.header().src = _ethernet_address;
    frame.header().type = EthernetHeader::TYPE_IPv4;
    frame.payload() = dgram.serialize();

    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    if (_known_addresses.count(next_hop_ip)) {
        const auto next_hop_ethernet_addr = _known_addresses[next_hop_ip].first;
        frame.header().dst = next_hop_ethernet_addr;
        _frames_out.push(frame);
    } else {
        _unresolved_frames[next_hop_ip].push(frame);

        if (_arp_in_flight_timestamp.count(next_hop_ip))
            return;

        ARPMessage request;
        request.opcode = ARPMessage::OPCODE_REQUEST;
        request.sender_ethernet_address = _ethernet_address;
        request.sender_ip_address = _ip_address.ipv4_numeric();
        request.target_ip_address = next_hop_ip;

        EthernetFrame arp_frame;
        arp_frame.header().dst = ETHERNET_BROADCAST;
        arp_frame.header().src = _ethernet_address;
        arp_frame.header().type = EthernetHeader::TYPE_ARP;
        arp_frame.payload() = request.serialize();

        _frames_out.push(arp_frame);
        _arp_in_flight_timestamp[next_hop_ip] = _age + ARP_REQUEST_TIMEOUT;
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    if (frame.header().dst != _ethernet_address && frame.header().dst != ETHERNET_BROADCAST)
        return {};

    if (frame.header().type == EthernetHeader::TYPE_IPv4) {
        InternetDatagram dgram;
        const auto res = dgram.parse(frame.payload());

        if (res == ParseResult::NoError)
            return dgram;
    } else if (frame.header().type == EthernetHeader::TYPE_ARP) {
        ARPMessage message;
        const auto res = message.parse(frame.payload());

        if (res != ParseResult::NoError)
            return {};

        if (message.opcode == ARPMessage::OPCODE_REQUEST) {
            resolve_ip(message.sender_ip_address, message.sender_ethernet_address);

            if (message.target_ip_address != _ip_address.ipv4_numeric())
                return {};

            ARPMessage reply;
            reply.opcode = ARPMessage::OPCODE_REPLY;
            reply.sender_ethernet_address = _ethernet_address;
            reply.sender_ip_address = _ip_address.ipv4_numeric();
            reply.target_ethernet_address = message.sender_ethernet_address;
            reply.target_ip_address = message.sender_ip_address;

            EthernetFrame arp_frame;
            arp_frame.header().dst = reply.target_ethernet_address;
            arp_frame.header().src = _ethernet_address;
            arp_frame.header().type = EthernetHeader::TYPE_ARP;
            arp_frame.payload() = reply.serialize();

            _frames_out.push(arp_frame);
        } else if (message.opcode == ARPMessage::OPCODE_REPLY)
            resolve_ip(message.sender_ip_address, message.sender_ethernet_address);
    }

    return {};
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) {
    _age += ms_since_last_tick;

    {
        auto it = _known_addresses.begin();
        while (it != _known_addresses.end()) {
            if (it->second.second <= _age)
                it = _known_addresses.erase(it);
            else
                it++;
        }
    }

    {
        auto it = _arp_in_flight_timestamp.begin();
        while (it != _arp_in_flight_timestamp.end()) {
            if (it->second <= _age)
                it = _arp_in_flight_timestamp.erase(it);
            else
                it++;
        }
    }
}
