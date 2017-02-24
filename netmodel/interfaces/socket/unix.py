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
from netmodel.util.misc         import silentremove

DEFAULT_PATH    = '/tmp/unix_interface'

class UnixProtocol:

    def send_impl(self, data):
        msg = data.encode()
        self._transport.write(msg)

    def data_received(self, data):
        """
        Called when some data is received.
        The argument is a bytes object.
        """
        msg = data.decode()
        self.receive(msg, ingress_interface=self)

class UnixServerProtocol(UnixProtocol, ServerProtocol, Interface):
    __interface__ = 'unix'

    def __init__(self, *args, **kwargs):
        # Note: super() does not call all parents' constructors
        Interface.__init__(self, *args, **kwargs)
        ServerProtocol.__init__(self)

class UnixClientProtocol(UnixProtocol, ClientProtocol):
    pass

#------------------------------------------------------------------------------

class UnixServerInterface(SocketServer, Interface):
    __interface__ = 'unixserver'

    def __init__(self, *args, **kwargs):
        SocketServer.__init__(self)
        self._path = kwargs.pop('path', DEFAULT_PATH)
        Interface.__init__(self, *args, **kwargs)

    def terminate(self):
        silentremove(self._path)

    def new_protocol(self):
        p = UnixServerProtocol(callback=self._callback, hook=self._hook)
        self.spawn_interface(p)
        return p

    def _create_socket(self):
        loop = asyncio.get_event_loop()
        silentremove(self._path)
        return loop.create_unix_server(self.new_protocol, self._path)

class UnixClientInterface(SocketClient, Interface):
    __interface__ = 'unixclient'

    def __init__(self, *args, **kwargs):
        SocketClient.__init__(self)
        self._path = kwargs.pop('path', DEFAULT_PATH)
        Interface.__init__(self, *args, **kwargs)

    def _create_socket(self):
        loop = asyncio.get_event_loop()
        protocol = lambda : UnixClientProtocol(self)
        return loop.create_unix_connection(protocol, self._path)
