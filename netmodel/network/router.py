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
import random
import string
import traceback

from netmodel.network.interface     import Interface, InterfaceState
from netmodel.network.fib           import FIB
from netmodel.network.flow_table    import FlowTable
from netmodel.network.packet        import Packet, ErrorPacket

log = logging.getLogger(__name__)

class Router:

    #--------------------------------------------------------------------------
    # Constructor, destructor, accessors
    #--------------------------------------------------------------------------

    def __init__(self, vicn_callback = None):
        """
        Constructor.
        Args:
            allowed_capabilities: A Capabilities instance which defines which
                operation can be performed by this Router. Pass None if there
                is no restriction.
        """
        # FIB
        self._fib = FIB()

        # Per-interface flow table
        self._flow_table = FlowTable()

        # interface_uuid -> interface
        self._interfaces = dict()

        self._vicn_callback = vicn_callback

    def terminate(self):
        for interface in self._interfaces.values():
            interface.terminate()

    # Accessors

    def get_fib(self):
        return self._fib

    #--------------------------------------------------------------------------
    # Collection management
    #--------------------------------------------------------------------------

    def register_local_collection(self, cls):
        self.get_interface(LOCAL_NAMESPACE).register_collection(cls,
                LOCAL_NAMESPACE)

    def register_collection(self, cls, namespace=None):
        self.get_interface(LOCAL_NAMESPACE).register_collection(cls, namespace)

    #--------------------------------------------------------------------------
    # Interface management
    #--------------------------------------------------------------------------

    def _register_interface(self, interface, name=None):
        if not name:
            name = 'interface-' + ''.join(random.choice(string.ascii_uppercase +
                        string.digits) for _ in range(3))
        interface.name = name
        self._interfaces[name] = interface

        # Populate interface callbacks
        interface.add_up_callback(self.on_interface_up)
        interface.add_down_callback(self.on_interface_down)
        interface.add_spawn_callback(self.on_interface_spawn)
        interface.add_delete_callback(self.on_interface_delete)

        log.info('Successfully created interface {} with name {}'.format(
                    interface.__interface__, name))

        interface.set_state(InterfaceState.PendingUp)

        for prefix in interface.get_prefixes():
            self._fib.add(prefix, set([interface]))

        return interface

    def _unregister_interface(self, interface):
        del self._interfaces[interface.name]

    # Interface events

    #--------------------------------------------------------------------------
    # Interface management
    #--------------------------------------------------------------------------

    def on_interface_up(self, interface):
        """
        This callback is triggered when an interface becomes up.

        The router will request metadata.
        The flow table is notified.
        """
        self._flow_table._on_interface_up(interface)

    def on_interface_down(self, interface):
        # We need to remove corresponding FIB entries
        log.info("Router interface is now down")

    def on_interface_spawn(self, interface):
        self._register_interface(interface)

    def on_interface_delete(self, interface):
        """Callback : an interface has been deleted.

        - TODO : purge the FIB
        - inform the flow table for managing pending subscriptions.
        """
        self._unregister_interface(interface)
        self._flow_table._on_interface_delete(interface)

    #---------------------------------------------------------------------------
    # Public API
    #---------------------------------------------------------------------------

    def add_interface(self, interface_type, name=None, namespace=None,
            **platform_config):
        """
        namespace is used to force appending of a namespace to the tables.
        existing namespaces are thus ignored.

        # This is the public facing interface, which internally uses
        # _register_interface.
        """
        interface_cls = Interface.get(interface_type)
        if interface_cls is None:
            log.warning("Could not create a %(interface_type)s interface" % \
                    locals())
            return None

        try:
            # passes a callback to the Interface
            # no hook
            platform_config['callback'] = self._on_receive
            interface = interface_cls(self, **platform_config)
        except Exception as e:
            traceback.print_exc()
            raise Exception("Cannot create interface %s of type %s with parameters %r: %s"
                    % (name, interface_type,
                    platform_config, e))
        self._register_interface(interface, name)
        return interface

    def is_interface_up(self, interface_name):
        interface = self._interfaces.get(interface_name)
        if not interface:
            return False
        return self._interfaces[interface_name].is_up()

    def del_platform(self, platform_name, rebuild = True):
        """
        Remove a platform from this Router. This platform is no more
        registered. The corresponding Announces are also removed.
        Args:
            platform_name: A String containing a platform name.
            rebuild: True if the DbGraph must be rebuild.
        Returns:
            True if it altered this Router.
        """
        ret = False
        try:
            del self._interfaces[platform_name]
            ret = True
        except KeyError:
            pass

        self.disable_platform(platform_name, rebuild)
        return ret

    def get_interface(self, platform_name):
        """
        Retrieve the Interface instance corresponding to a platform.
        Args:
            platform_name: A String containing the name of the platform.
        Raises:
            ValueError: if platform_name is invalid.
            RuntimeError: in case of failure.
        Returns:
            The corresponding Interface if found, None otherwise.
        """
        if platform_name.lower() != platform_name:
            raise ValueError("Invalid platform_name = %s, must be lower case" \
                    % platform_name)

        if platform_name in self._interfaces:
            return self._interfaces[platform_name]

        raise RuntimeError("%s is not yet registered" % platform_name)

    def get_interface_names(self):
        return self._interfaces.keys()

    def get_interfaces(self):
        return self._interfaces.values()

    #--------------------------------------------------------------------------
    # Packet operations
    #--------------------------------------------------------------------------

    def _on_receive(self, packet, ingress_interface):
        """Handles reception of a new packet.

        An incoming packet is forwarder either:
         - using the reverse path is there is a match with the ingress
           interface flow table
         - using the FIB if no match is found
        """
        orig_interfaces = self._flow_table.match(packet, ingress_interface)
        if orig_interfaces:
            for orig_interface in orig_interfaces:
                orig_interface.send(packet)
            return

        if isinstance(packet, str):
            # Workaround : internal command
            if self._vicn_callback:
                self._vicn_callback(packet)
            return

        if packet.source is None and packet.destination is None:
            log.warning('TODO: handle NULL packet, need source on all packets')
            return

        # Get route from FIB
        if packet.destination is None:
            log.warning("Ignored reply packet with no match in flow table {}".format(
                        packet.to_query()))
            return
        interfaces = self._fib.get(packet.destination.object_name)
        if not interfaces:
            log.error('No match in FIB for {}'.format(
                        packet.destination.object_name))
            return

        # Update flow table before sending
        self._flow_table.add(packet, ingress_interface, interfaces)
        for interface in interfaces:
            interface.send(packet)
