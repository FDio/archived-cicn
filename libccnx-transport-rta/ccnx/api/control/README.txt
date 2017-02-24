/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

Control Plane Interface (CPI)

Control's the FIB and forwarder interfaces.

Also controls stack behavior, such as flushing the stack.

These messages are meant to be sent up and down transport connections, not
over the command port.

The matieral below describes the networking model used by the Transport.

=====================================

Most of the API functions are found in cpi_ManageLinks.h and cpi_Forwarding.h.  Those
have the calls to generate CPIControlMessages for dealing with interfaces and dealing with
forwarding.

=====================================
0) Overview

To use the ControlPlaneInterface, you create CPIControlMessages, then send them
down a Transport connection to a forwarder.  The Forwarder Connector, in conjunction
with the forwarder will send a CPIControlMessage back up the stack to you.  The
Response might be an ACK, a NACK, or a Respone with data.

Different Transports and different forwarders will have their own models of
ports, interfaces, and addresses.  It is the job of the Transport and the
Forwarder Connector to map those different views in to the single consistent
view exposed here to the API.

All addressable entities have an "Interface Index" (ifidx), similar to
RFC 3493, Section 4.  The interface index used by the Transport includes
virtual interfaces that may not exist at the kernel level, so the ifidx
is not equal to a kernel interface index.  For some devices, the CCNx
forwarder might not even be on the same physical entity.

Not all forwarders support all options.  See forwarder specific documentation.

=====================================
1) Interfaces

An interface is a physical port or virtual device on the system.
It may have zero or more addresses, depending on the type.

- P2P interface (e.g. serial)
- Ethernet
- Loopback

Some interfaces depend on other interfaces:

- VLAN
- L2TP
- PPP
- LAG

Interfaces will have a layer 2 (L2) address and may have L3 addresses too.

All physical and virtual interfaces have an interface index.

=====================================
2) Overlay Interfaces (Tunnels and Multicast groups)

Tunnels are a special type of Interface that have a point-to-point
connection with a remote peer.  Another type of overlay is
an IP multicast group.

Tunnels have a specific local source address.  Each tunnel and multicast
group overlay has an interface index.

Type types of interaces are:
- IP/UDP   point-to-point tunnel
- IPv6/UDP point-to-point tunnel
- IP/TCP   point-to-point tunnel
- IPv6/TCP point-to-point tunnel
- IP/UDP   multicast group
- IPv6/UDP multicast group


=====================================
3) Addresses

Addresses are used for setting up tunnels and overlays.  They
are also used in the FIB to indicate a next hop.

An Interface address may be used with tunnels and overlays to
indicate that a message (i.e. interest) should be sent on that
overlay.

- IPv4 unicast
- IPv4 multicast
- IPv6 unicast
- IPv6 multicast
- Link unicast
- Link group
- Interface

=====================================
4) Message information

Similar to RFC 3542, Section 6

An outgoing message can specify:
- The outgoing interface index
- The outgoing hop limit

If the outgoing interface is specified, it bypasses the normal
FIB/PIT forwarding rules and forces the message out that interface.


Incoming message information may contain:
- The destination address
- The arriving interface index
- The arriving hop limit

There is currently no specification for traffic class.

The destination address will be specific to how the message was
received.  If it was over an IP-based interface, the addresses will
be IP/IPv6.  If it was over a Link interface, the addresses wil be
media-dependent addresses (e.g. Ethernet MACs).

To control the incoming information, use these functions
to generate a control message to send down the connection.
They are similar to an IP(V6)_RECVPKTINFO socket option.

CPIControlMessage * cpi_StartReceivingMessageInfo();
CPIControlMessage * cpi_StopReceivingMessageInfo();

DESCRIBE HOW IT IS COMMUNICATED IN A CCNxMESSAGE

=====================================
5) Monitors

A Transport connection can be setup as a Monitor.  A Montior is
an INBOUND ONLY connection for diagnostic purposes.  It should
contain only the minimum necessary components (API, CODEC, Forwarder).

A Monitor has directionality.  It may snoop all messages INBOUND or OUTBOUND
or BOTH on the target.  The snooped messages are sent up the monitor
connection to the API.

A Monitor may be added to an Interface, in which case it will act like
a promiscuous tap.  An INBOUND monitor will see all packets received
on the interface.  An OUTBOUND monitor will see all packets sent on the
interface.

A Monitor may be added to a namespace.  It can be for an exact namespace,
or it can be a prefix match.  It may be INBOUND or OUTBOUND or BOTH.

=====================================
Common Operations

####
#### NOTE: This example code is out of date.
####

----------------------------------------------------------------------------
a) List the interfaces

CPIControlMessage * cpi_NetworkInterfaceList();
/* send the control message on a connection */

CPIControlMessage *message = /* receive function */
if( cpi_IsCpiMessage(message) && cpi_GetMessageType(message) == CPI_RESPONSE &&
    cpi_GetMessageOperation(message) == CPI_INTERFACE_LIST )
{
    unsigned count = cpi_NetworkInterfaceList_Count(message);
    for(i = 0; i < count; i++)
    {
        InterfaceService *entry = cpi_NetworkInterfaceList_Get(message, i);
    }
}

ccnxControlMessage_Destroy(&message);


----------------------------------------------------------------------------
b) Create a point-to-point tunnel

CPIAddress * dest = cpiAddress_CreateFromInet((struct sockaddr_in) {
                                     .sa_addr = inet_addr("foo.bar.com"),
                                     .sa_port = htons(9695)} );

// the address 13.0.1.1. is known to be on the forwarder.
// You'd learn about it from cpi_NetworkInterfaceList()
CPIAddress * source = cpiAddress_CreateFromInet((struct sockaddr_in) {
                                     .sa_family = AF_INET,
                                     .sa_addr = inet_addr("13.0.1.1") } );

CPIControlMessage *udp_tunnel = cpiTunnel_CreateMessage(dest, source, IPPROTO_UDP)
/* send message down connection */

CPIControlMessage *response = /* receive message function */

if( cpi_IsCpiMessage(response) && cpi_GetMessageType(response) == CPI_RESPONSE &&
    cpi_GetMessageOperation(response) == CPI_CREATE_TUNNEL )
{
    CPITunnel * tunnel = cpiTunnel_ParseMessage(response);

    unsigned ifidx = cpiTunnel_GetInterfaceIndex(tunnel);
    // ...
    cpiTunnel_Destroy(&tunnel);
}



----------------------------------------------------------------------------
c) Create an IP multicast overlay

CPIAddress * dest = cpiAddress_CreateFromInet((struct sockaddr_in) {
                                     .sa_family = AF_INET,
                                     .sa_addr = inet_addr("224.1.100.3"),
                                     .sa_port = htons(9695)} );

// the address 13.0.1.1. is known to be on the forwarder.
// You'd learn about it from cpi_NetworkInterfaceList()
CPIAddress * source = cpiAddress_CreateFromInet((struct sockaddr_in) {
                                     .sa_family = AF_INET,
                                     .sa_addr = inet_addr("13.0.1.1") } );

unsigned ifidx = /* interface to join the group */

CPIControlMessage *udp_multicast = cpiMulticast_CreateMessage(dest, source, ifidx)
/* send message down connection */

CPIControlMessage *response = /* receive message function */

if( cpi_IsCpiMessage(response) && cpi_GetMessageType(response) == CPI_RESPONSE &&
    cpi_GetMessageOperation(response) == CPI_CREATE_MULTICAST )
{
    CPIMulticast * mcast = cpiMulticast_ParseMessage(response);
    // ...
    cpiMulticast_Destroy(&tunnel);
}

----------------------------------------------------------------------------
d) Create an Ethernet group interface




----------------------------------------------------------------------------
d) remove a tunnel

----------------------------------------------------------------------------
d) Setup a FIB entry to a point-to-point tuennl

----------------------------------------------------------------------------
e) Setup a FIB entry to a multicast group

----------------------------------------------------------------------------
f) Setup a FIB entry to a Link unicast address

----------------------------------------------------------------------------
g) Setup a FIB entry to a Link group address




