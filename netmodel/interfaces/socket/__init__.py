#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2017 Cisco and/or its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import asyncio
import json

from netmodel.network.packet      import Packet
from netmodel.network.interface   import Interface, InterfaceState

class Protocol(asyncio.Protocol):
    def __init__(self):
        self._transport = None

    def terminate(self):
        if self._transport:
            self._transport.close()

    def send(self, packet):
        if isinstance(packet, Packet):
            data = json.dumps(packet.to_query().to_dict())
        else:
            data = packet 
        self.send_impl(data)

    def receive(self, data, ingress_interface):
        try:
            packet = Packet.from_query(Query.from_dict(json.loads(data)))
        except:
            packet = data 
        self.receive(packet, ingress_interface)


class ServerProtocol(Protocol):
    # asyncio.Protocol

    def __init__(self):
        super().__init__()

    def connection_made(self, transport):
        """
        Called when a connection is made.
        The argument is the _transport representing the pipe connection.
        To receive data, wait for data_received() calls.
        When the connection is closed, connection_lost() is called.
        """
        self._transport = transport
        self.set_state(InterfaceState.Up)

    def connection_lost(self, exc):
        """
        Called when the connection is lost or closed.
        The argument is an exception object or None (the latter
        meaning a regular EOF is received or the connection was
        aborted or closed).
        """
        self.set_state(InterfaceState.Down)

class ClientProtocol(Protocol):
    def __init__(self, interface, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._interface = interface

    # asyncio.Protocol

    def connection_made(self, transport):
        """
        Called when a connection is made.
        The argument is the _transport representing the pipe connection.
        To receive data, wait for data_received() calls.
        When the connection is closed, connection_lost() is called.
        """
        self._transport = transport
        self._interface.set_state(InterfaceState.Up)

    def connection_lost(self, exc):
        """
        Called when the connection is lost or closed.
        The argument is an exception object or None (the latter
        meaning a regular EOF is received or the connection was
        aborted or closed).
        """
        self._interface.set_state(InterfaceState.Down)



#------------------------------------------------------------------------------

class SocketServer:
    def __init__(self, *args, **kwargs):
        # For a server, an instance of asyncio.base_events.Server
        self._transport = None
        self._clients = list()

    def terminate(self):
        """Close the server and terminate all clients.
        """
        self._socket.close()
        for client in self._clients:
            self._clients.terminate()

    def send(self, packet):
        """Broadcast packet to all connected clients.
        """
        for client in self._clients:
            self._clients.send(packet)

    def receive(self, packet):
        raise RuntimeError('Unexpected packet received by server interface')

    def __repr__(self):
        if self._transport:
            peername = self._transport.get_extra_info('peername')
        else:
            peername = 'not connected'
        return '<Interface {} {}>'.format(self.__interface__, peername)

    async def pending_up_impl(self):
        try:
            self._server = await self._create_socket()
        except Exception as e:
            await self._set_state(InterfaceState.Error)
            self._error = str(e)

        # Only the server interface is up once the socket has been created and
        # is listening...
        await self._set_state(InterfaceState.Up)

class SocketClient:
    def __init__(self, *args, **kwargs):
        # For a client connection, this is a tuple 
        # (_SelectorSocketTransport, protocol)
        self._transport = None 
        self._protocol = None

    def send_impl(self, packet):
        self._protocol.send(packet)

    async def pending_up_impl(self):
        try:
            self._transport, self._protocol = await self._create_socket()
        except Exception as e:
            await self._set_state(InterfaceState.Error)
            self._error = str(e)

    def pending_down_impl(self):
        if self._socket:
            self._transport.close()

    def __repr__(self):
        if self._socket:
            peername = self._transport.get_extra_info('peername')
        else:
            peername = 'not connected'
        return '<Interface {} {}>'.format(self.__interface__, peername)


