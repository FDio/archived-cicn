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

import ipaddress
import logging
import random
import struct
import socket

from netmodel.util.deprecated   import deprecated
from netmodel.util.singleton    import Singleton
from netmodel.util.toposort     import toposort, toposort_flatten

log = logging.getLogger(__name__)

#------------------------------------------------------------------------------
# SharedResource
#------------------------------------------------------------------------------

class SharedResource:
    """
    Base class for allocating shared resource
    """
    def __init__(self):
        self._counter = 0
        self._values = dict()

    def __next__(self):
        ret = self._counter
        self._counter += 1
        return ret

    def get(self, requestor, tag = None):
        if requestor not in self._values:
            self._values[requestor] = dict()
        if tag not in self._values[requestor]:
            self._values[requestor][tag] = next(self)
        return self._values[requestor][tag]

#------------------------------------------------------------------------------

class Vlan(SharedResource):
    """
    SharedResource: Vlan

    Manages VLAN allocation
    """
    
    def get(self, requestor, tag = None):
        if requestor not in self._values:
            self._values[requestor] = dict()
        if tag not in self._values[requestor]:
            self._values[requestor][tag] = (next(self)+1)
        return self._values[requestor][tag]

#------------------------------------------------------------------------------
    
MAX_DEVICE_NAME_SIZE = 15

class DeviceName(SharedResource):

    def get(self, *args, prefix = None, **kwargs):
        count = super().get(*args, **kwargs)
        device_name = '{}{}'.format(prefix, count)
        if len(device_name) > MAX_DEVICE_NAME_SIZE:
            overflow = len(device_name) - MAX_DEVICE_NAME_SIZE
            max_prefix_len = len(prefix) - overflow
            device_name = '{}{}'.format(prefix[:max_prefix_len], count)
        return device_name

#------------------------------------------------------------------------------

class IpAddress(SharedResource):
    pass

class Ipv4Address(IpAddress):
    pass

class Ipv6Address(IpAddress):
    pass

#------------------------------------------------------------------------------
# AddressManager
#------------------------------------------------------------------------------

class AddressManager(metaclass = Singleton):
    """
    The purpose of this class is to generate sequential deterministic MAC/IP
    addresses in order to assign them to the node in the network.
    """

    MAP_TYPE = {
        'vlan': Vlan,
        'device_name': DeviceName,
    }

    def __init__(self):
        self._ips = dict()
        self._macs = dict()

        self._pools = dict()

        from vicn.core.resource_mgr import ResourceManager

        network = ResourceManager().get('network')
        network = ipaddress.ip_network(network, strict=False)
        self._next_ip = network[1]

        mac_address_base = ResourceManager().get('mac_address_base')
        self._next_mac = int(mac_address_base, 0) + 1

    def get(self, resource_type, requestor, *args, tag=None, scope=None,
            **kwargs):
        """
        Params:
          type : the type of shared resource to be requested
          requestor: name of the resource that requests the shared resource, in
            order to reattribute the same if requested multiple times.
          tag: use when a single resource request multiple times the same
            resource.
          scope: None = global scope by default. Ensure uniqueness of resource
            at global scope
        """
        if not scope in self._pools:
            self._pools[scope] = dict()
        if not resource_type in self._pools[scope]:
            self._pools[scope][resource_type] = self.MAP_TYPE[resource_type]() 
        return self._pools[scope][resource_type].get(requestor, tag, 
                *args, **kwargs)

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def get_mac(self, resource_name):
        """
        Generate a new mac address to assign to the containers created.

        :return: The MAC address
        """

        if resource_name in self._macs:
            return self._macs[resource_name]

        mac = ':'.join(map(''.join, 
                    zip(*[iter(hex(self._next_mac)[2:].zfill(12))]*2)))
        self._next_mac += 1

        self._macs[resource_name] = mac
        return mac

    def get_ip(self, resource):
        """
        Generate a new ip address to assign to the containers created.

        :return: The IP address
        """

        if resource in self._ips:
            return self._ips[resource]

        ip = str(self._next_ip)
        self._next_ip += 1

        self._ips[resource] = ip
        return ip
