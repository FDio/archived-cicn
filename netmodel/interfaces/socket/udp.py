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

from netmodel.interfaces.socket import ServerProtocol, ClientProtocol
from netmodel.interfaces.socket import SocketClient, SocketServer
from netmodel.network.interface import Interface

DEFAULT_ADDRESS = '127.0.0.1'
DEFAULT_PORT    = 7000

class UDPProtocol:

    def send_impl(self, data):
        msg = data.encode()
        self._transport.sendto(msg)

    # asyncio.Protocol

    def datagram_received(self, data, addr):
        msg = data.decode()
        self.receive(msg, ingress_interface=self)

    def error_received(self, exc):
        print('Error received:', exc)

class UDPServerProtocol(UDPProtocol, ServerProtocol, Interface):
    __interface__ = 'udp'

    def __init__(self, *args, **kwargs):
        # Note: super() does not call all parents' constructors
        Interface.__init__(self, *args, **kwargs)
        ServerProtocol.__init__(self)

class UDPClientProtocol(UDPProtocol, ClientProtocol):
    pass

#------------------------------------------------------------------------------

class UDPServerInterface(SocketServer, Interface):
    __interface__ = 'udpserver'

    def __init__(self, *args, **kwargs):
        SocketServer.__init__(self)
        self._address = kwargs.pop('address', DEFAULT_ADDRESS)
        self._port = kwargs.pop('port', DEFAULT_PORT)
        Interface.__init__(self, *args, **kwargs)

    def new_protocol(self):
        p = UdpServerProtocol(callback = self._callback, hook=self._hook)
        self.spawn_interface(p)
        return p

    def _create_socket(self):
        loop = asyncio.get_event_loop()
        return loop.create_datagram_endpoint(self.new_protocol,
                local_addr=(self._address, self._port))

class UDPClientInterface(SocketClient, Interface):
    __interface__ = 'udpclient'

    def __init__(self, *args, **kwargs):
        SocketClient.__init__(self)
        self._address = kwargs.pop('address', DEFAULT_ADDRESS)
        self._port = kwargs.pop('port', DEFAULT_PORT)
        Interface.__init__(self, *args, **kwargs)

    def _create_socket(self):
        loop = asyncio.get_event_loop()
        protocol = lambda : UDPClientProtocol(self)
        return loop.create_datagram_endpoint(protocol,
                remote_addr=(self._address, self._port))
