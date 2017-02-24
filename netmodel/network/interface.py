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
import enum
import inspect
import logging
import pkgutil
import sys
import traceback

import netmodel.interfaces as interfaces
from netmodel.network.packet           import Packet
from netmodel.network.prefix           import Prefix

# Tag identifying an interface name
INTERFACE_NAME_PROPERTY = '__interface__'

RECONNECTION_DELAY = 10

log = logging.getLogger(__name__)

class InterfaceState(enum.Enum):
    Up          = 'up'
    Down        = 'down'
    PendingUp   = 'up (pending)'
    PendingDown = 'down (pending)'
    Error       = 'error'

def register_interfaces():
    Interface._factory = dict()
    for loader, module_name, is_pkg in pkgutil.walk_packages(interfaces.__path__, 
            interfaces.__name__ + '.'):
        # Note: we cannot skip files using is_pkg otherwise it will ignore
        # interfaces defined outside of __init__.py
        #if not is_pkg:
        #    continue
        try:
            module = loader.find_module(module_name).load_module(module_name)
            for _, obj in inspect.getmembers(module):
                if not inspect.isclass(obj):
                    continue
                if not issubclass(obj, Interface):
                    continue
                if obj is Interface:
                    continue
                if not hasattr(obj, INTERFACE_NAME_PROPERTY):
                    log.warning('Ignored interface' + module_name + \
                            'with no ' + INTERFACE_NAME_PROPERTY + ' property')
                    continue
                name = getattr(obj, INTERFACE_NAME_PROPERTY)
                Interface._factory[name] = obj

        except ImportError as e:
            log.warning('Interface {} automatically disabled. ' \
                    'Please install dependencies if you wish to use it: {}'\
                    .format(module_name, e))

        except Exception as e:
            log.warning('Failed to load interface {}: {}'.format(
                        module_name, e))

#------------------------------------------------------------------------------

class Interface:
    @classmethod
    def get(cls, name):
        if not hasattr(cls, '_factory'):
            register_interfaces()
        return cls._factory.get(name, None)

    STATE_DOWN = 0
    STATE_PENDING_UP = 1
    STATE_UP = 2
    STATE_PENDING_DOWN = 3

    def __init__(self, *args, **kwargs):
        self._callback = kwargs.pop('callback', None)
        self._hook = kwargs.pop('hook', None)

        self._tx_buffer = list()
        self._state    = InterfaceState.Down
        self._error    = None 
        self._reconnecting = True
        self._reconnection_delay = RECONNECTION_DELAY

        self._registered_objects = dict()
    
        # Callbacks
        self._up_callbacks   = list()
        self._down_callbacks = list()
        self._spawn_callbacks = list()
        self._delete_callbacks = list()

        # Set upon registration
        self._name = None

    def terminate(self):
        self.set_state(InterfaceState.PendingDown)

    def __repr__(self):
        return "<Interface %s>" % (self.__class__.__name__)

    def __hash__(self):
        return hash(self._name)

    #--------------------------------------------------------------------------- 

    def register_object(self, obj):
        self._registered_objects[obj.__type__] = obj

    def get_prefixes(self):
        return [ Prefix(v.__type__) for v in self._registered_objects.values() ]

    #--------------------------------------------------------------------------- 
    # State management, callbacks
    #--------------------------------------------------------------------------- 

    def set_state(self, state):
        asyncio.ensure_future(self._set_state(state))

    async def _set_state(self, state):
        self._state = state

        if state == InterfaceState.PendingUp:
            await self.pending_up_impl()
        elif state == InterfaceState.PendingDown:
            await self.pending_down_impl()
        elif state == InterfaceState.Error:
            pass
        elif state == InterfaceState.Up:
            log.info("Interface {} : new state UP.".format(self.__interface__,))
            if self._tx_buffer:
                log.info("Platform %s: sending %d buffered packets." %
                        (self.__interface__, len(self._tx_buffer)))
            while self._tx_buffer:
                packet = self._tx_buffer.pop()
                self.send_impl(packet)
            # Trigger callbacks to inform interface is up
            for cb, args, kwargs in self._up_callbacks:
                cb(self, *args, **kwargs)
        elif state == InterfaceState.Down:
            log.info("Interface %s: new state DOWN." % (self.__interface__,))
            self._state = self.STATE_DOWN
            # Trigger callbacks to inform interface is down
            for cb, args, kwargs in self._down_callbacks:
                cb(self, *args, **kwargs)

    def spawn_interface(self, interface):
        #print('spawn interface', interface)
        for cb, args, kwargs in self._spawn_callbacks:
            cb(interface, *args, **kwargs)

    def delete_interface(self, interface):
        for cb, args, kwargs in self._delete_callbacks:
            cb(interface, *args, **kwargs)

    #-------------------------------------------------------------------------- 

    def set_reconnecting(self, reconnecting):
        self._reconnecting = reconnecting

    def get_interface_type(self):
        return self.__interface__

    def get_description(self):
        return str(self)

    def get_status(self):
        return 'UP' if self.is_up() else 'ERROR' if self.is_error() else 'DOWN'

    def is_up(self):
        return self._state == InterfaceState.Up

    def is_down(self):
        return not self.is_up()

    def is_error(self):
        return self.is_down() and self._error is not None

    def reinit_impl(self):
        pass

    def reinit(self, **platform_config):
        self.set_down()
        if platform_config:
            self.reconnect_impl(self, **platform_config)
        self.set_up()

    #--------------------------------------------------------------------------
    # Callback management
    #--------------------------------------------------------------------------

    def add_up_callback(self, callback, *args, **kwargs):
        cb_tuple = (callback, args, kwargs)
        self._up_callbacks.append(cb_tuple)

    def del_up_callback(self, callback):
        self._up_callbacks = [cb for cb in self._up_callbacks \
                if cb[0] == callback]

    def add_down_callback(self, callback, *args, **kwargs):
        cb_tuple = (callback, args, kwargs)
        self._down_callbacks.append(cb_tuple)

    def del_down_callback(self, callback):
        self._down_callbacks = [cb for cb in self._down_callbacks \
                if cb[0] == callback]

    def add_spawn_callback(self, callback, *args, **kwargs):
        cb_tuple = (callback, args, kwargs)
        self._spawn_callbacks.append(cb_tuple)

    def del_spawn_callback(self, callback):
        self._spawn_callbacks = [cb for cb in self._spawn_callbacks \
                if cb[0] == callback]

    def add_delete_callback(self, callback, *args, **kwargs):
        cb_tuple = (callback, args, kwargs)
        self._delete_callbacks.append(cb_tuple)

    def del_delete_callback(self, callback):
        self._delete_callbacks = [cb for cb in self._delete_callbacks \
                if cb[0] == callback]

    #--------------------------------------------------------------------------
    # Interface API
    #--------------------------------------------------------------------------

    async def pending_up_impl(self):
        self.set_state(InterfaceState.Up)

    def send_impl(self, packet):
        query = packet.to_query()
        obj = self._registered_objects.get(query.object_name)
        obj.get(query, self)

    def receive_impl(self, packet):
        ingress_interface = self
        cb = self._callback
        if cb is None:
            return
        if self._hook:
            new_packet = self._hook(packet)
            if new_packet is not None:
                cb(new_packet, ingress_interface=ingress_interface)
            return
        cb(packet, ingress_interface=ingress_interface)

    #--------------------------------------------------------------------------

    def send(self, packet):
        if self.is_up():
            self.send_impl(packet)
        else:
            self._tx_buffer.append(packet)

    def receive(self, packet):
        """
        For packets received from outside (eg. a remote server).
        """
        self.receive_impl(packet)

    def execute(self, query):
        self.send(Packet.from_query(query))

