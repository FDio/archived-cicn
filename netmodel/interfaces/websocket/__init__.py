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
import logging
import json

from netmodel.network.interface import Interface, InterfaceState
from netmodel.network.packet    import Packet
from netmodel.model.query       import Query
from netmodel.model.query       import ACTION_INSERT, ACTION_SELECT
from netmodel.model.query       import ACTION_UPDATE, ACTION_DELETE
from netmodel.model.query       import ACTION_EXECUTE

from autobahn.asyncio.websocket import WebSocketClientProtocol, \
    WebSocketClientFactory
from autobahn.asyncio.websocket import WebSocketServerProtocol, \
    WebSocketServerFactory

log = logging.getLogger(__name__)

DEFAULT_ADDRESS = '0.0.0.0'
DEFAULT_CLIENT_ADDRESS = '127.0.0.1'
DEFAULT_PORT = 9000
DEFAULT_TIMEOUT = 2

#------------------------------------------------------------------------------

from json import JSONEncoder
class DictEncoder(JSONEncoder):
    """Default JSON encoder

    Because some classes are not JSON serializable, we define here our own
    encoder which is based on the member variables of the object.

    The ideal solution would be to make all objects JSON serializable, but this
    encoder is useful for user-defined classes that would otherwise make the
    whole program to fail. It might though raise a warning to incitate
    developers to make their class conformant.

    Reference:
    http://stackoverflow.com/questions/3768895/how-to-make-a-class-json-serializable
    """
    def default(self, o):
        try:
            return vars(o)
        except:
            return {}

#------------------------------------------------------------------------------

class ClientProtocol(WebSocketClientProtocol):
    """
    Default WebSocket client protocol.

    This protocol is mainly used to relay events to the Interface, which is
    pointer to by the factory.
    """

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def send_impl(self, packet):
        msg = json.dumps(packet.to_query().to_dict())
        self.sendMessage(msg.encode(), False)

    #--------------------------------------------------------------------------
    # WebSocket events
    #--------------------------------------------------------------------------

    # Websocket events

    def onConnect(self, response):
        """
        Websocket opening handshake is started by the client.
        """
        self.factory.interface._on_client_connect(self, response)

    def onOpen(self):
        """
        Websocket opening handshake has completed.
        """
        self.factory.interface._on_client_open(self)
    
    def onMessage(self, payload, isBinary):
        self.factory.interface._on_client_message(self, payload, isBinary)

    def onClose(self, wasClean, code, reason):
        self.factory.interface._on_client_close(self, wasClean, code, reason)

#------------------------------------------------------------------------------

class WebSocketClientInterface(Interface):
    """
    All messages are exchanged using text (non-binary) mode.
    """   
    __interface__ = 'websocketclient'

    def __init__(self, *args, **kwargs):
        """
        Constructor.

        Args:
            address (str) : Address of the remote websocket server. Defaults to
                127.0.0.1 (localhost).
            port (int) : Port of the remote websocket server. Defaults to 9999.

        This constructor triggers the initialization of a WebSocket client
        factory, which is associated a ClientProtocol, as well as a reference
        to the current interface.

        PendingUp --- connect --- Up   ...disconnect... Down
                         A     |
                         +-----+
                          retry

        All messages are exchanged using text (non-binary) mode.
        """

        self._address = kwargs.pop('address', DEFAULT_CLIENT_ADDRESS)
        self._port    = kwargs.pop('port', DEFAULT_PORT)
        self._timeout = kwargs.pop('timeout', DEFAULT_TIMEOUT)

        super().__init__(*args, **kwargs)

        self._factory = WebSocketClientFactory("ws://{}:{}".format(
                    self._address, self._port))
        self._factory.protocol = ClientProtocol
        self._factory.interface = self

        self._instance = None

        # Holds the instance of the connect client protocol
        self._client = None

    #--------------------------------------------------------------------------
    # Interface API
    #--------------------------------------------------------------------------

    async def pending_up_impl(self):
        await self._connect()

    def send_impl(self, packet):
        if not self._client:
            log.error('interface is up but has no client')
        self._client.send_impl(packet)

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    async def _connect(self):
        loop = asyncio.get_event_loop()
        try:
            self._instance = await loop.create_connection(self._factory, 
                    self._address, self._port)
        except Exception as e:
            log.warning('Connect failed : {}'.format(e))
            self._instance = None
            # don't await for retry, since it cause an infinite recursion...
            asyncio.ensure_future(self._retry())

    async def _retry(self):
        """
        Timer: retry connection after timeout.
        """
        log.info('Reconnecting in {} seconds...'.format(self._timeout))
        await asyncio.sleep(self._timeout)
        log.info('Reconnecting...')
        await self._connect()

    # WebSocket events (from the underlying protocol)

    def _on_client_connect(self, client, response):
        self._client = client

    def _on_client_open(self, client):
        self.set_state(InterfaceState.Up)

    def _on_client_message(self, client, payload, isBinary):
        """
        Event: a message is received by the WebSocket client connection.
        """

        assert not isBinary

        args = json.loads(payload.decode('utf-8'))
        query, record = None, None
        if len(args) == 2:
            query, record = args
        else:
            query = args
        
        if isinstance(query, dict):
            query = Query.from_dict(query)
        else:
            query = Query(ACTION_SELECT, query)

        packet = Packet.from_query(query)

        self.receive(packet)

    def _on_client_close(self, client, wasClean, code, reason):
        self._client = None
        self._instance = None

        self.set_state(InterfaceState.Down)

        # Schedule reconnection
        asyncio.ensure_future(self._retry())

#------------------------------------------------------------------------------
        
class ServerProtocol(WebSocketServerProtocol, Interface):
    """
    Default WebSocket server protocol.

    This protocol is used for every server-side accepted WebSocket connection.
    As such it is an Interface on its own, and should handle the Interface state
    machinery.

    We would better triggering code directly
    """
    __interface__ = 'websocket'

    def __init__(self, callback, hook):
        """
        Constructor.

        Args:
            callback (Function[ -> ]) : 
            hook (Function[->]) : Hook method to be called for every packet to
                be sent on the interface. Processing continues with the packet
                returned by this function, or is interrupted in case of a None
                value. Defaults to None = no hook.
        """
        WebSocketServerProtocol.__init__(self)
        Interface.__init__(self, callback=callback, hook=hook)

    #--------------------------------------------------------------------------
    # Interface API
    #--------------------------------------------------------------------------

    async def pending_up_impl(self):
        await self._set_state(InterfaceState.Up)

    def send_impl(self, packet):
        # We assume we only send records...
        msg = json.dumps(packet.to_query().to_dict(), cls=DictEncoder)
        self.sendMessage(msg.encode(), False)

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    # Websocket events

    def onConnect(self, request):
        self.factory._instances.append(self)
        self.set_state(InterfaceState.Up)
        
    def onOpen(self):
        #print("WebSocket connection open.")
        pass

    def onMessage(self, payload, isBinary):
        assert not isBinary, "Binary message received: {0} bytes".format(
                len(payload))
        query_dict = json.loads(payload.decode('utf8'))
        query = Query.from_dict(query_dict)
        packet = Packet.from_query(query)
        self.receive(packet)

    def onClose(self, wasClean, code, reason):
        self.set_state(InterfaceState.Down)
        try:
            self.factory._instances.remove(self)
        except: pass

        self.delete_interface(self)

#------------------------------------------------------------------------------

class WebSocketServerInterface(Interface):
    """
    This virtual interface only listens for incoming connections in order to
    dynamically instanciate new interfaces upon client connection.

    It is also used to broadcast packets to all connected clients.

    All messages are exchanged using text (non-binary) mode.
    """   

    __interface__ = 'websocketserver'

    def __init__(self, *args, **kwargs):
        self._address = kwargs.pop('address', DEFAULT_ADDRESS)
        self._port    = kwargs.pop('port',    DEFAULT_PORT)

        super().__init__(*args, **kwargs)

        def new_server_protocol():
            p = ServerProtocol(self._callback, self._hook)
            self.spawn_interface(p)
            return p

        ws_url = u"ws://{}:{}".format(self._address, self._port)
        self._factory = WebSocketServerFactory(ws_url)
        # see comment in MyWebSocketServerFactory
        self._factory.protocol = new_server_protocol
        self._factory._callback = self._callback
        self._factory._interface = self

        # A list of all connected instances (= interfaces), used to broadcast
        # packets.
        self._factory._instances = list()

    #--------------------------------------------------------------------------
    # Interface API
    #--------------------------------------------------------------------------

    async def pending_up_impl(self):
        """
        As we have no feedback for when the server is actually started, we mark
        the interface up as soon as the create_server method returns.
        """
        loop = asyncio.get_event_loop()
        # Websocket server
        log.info('WebSocket server started')
        self._server = await loop.create_server(self._factory, self._address, 
                self._port)
        await self._set_state(InterfaceState.Up)

    async def pending_down_impl(self):
        raise NotImplementedError

    def send_impl(self, packet):
        """
        Sends a packet to all connected clients (broadcast).
        """
        for instance in self._factory._instances:
            instance.send(packet)
