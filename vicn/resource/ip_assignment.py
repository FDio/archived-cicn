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

import math
import logging

from vicn.core.resource                 import Resource
from netmodel.model.type                import String
from vicn.core.attribute                import Attribute
from vicn.resource.ip.prefix_tree       import Inet6Prefix, PrefixTree, Inet4Prefix
from vicn.core.task                     import inline_task, async_task, EmptyTask
from vicn.core.exception                import ResourceNotFound

log = logging.getLogger(__name__)

class IpAssignment(Resource):
    prefix = Attribute(String, mandatory=True)
    control_prefix = Attribute(String, description="prefix for control plane")
    max_prefix_size = Attribute(String,
            description="Maximum assigned prefix size for a link")

    PrefixClass = None

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._prefix = self.PrefixClass(self.prefix)
        self._prefix_tree = PrefixTree(self._prefix)
        self._assigned_prefixes = {}
        if not self.max_prefix_size:
            self.max_prefix_size = self.PrefixClass.MAX_PREFIX_SIZE
        if self.control_prefix:
            self._ctrl_prefix = self.PrefixClass(self.control_prefix)
            self._ctrl_prefix_it = iter(self._ctrl_prefix)
            next(self._ctrl_prefix_it) #Removes internet address
            self._assigned_addresses = {}

    def get_prefix(self, obj, prefix_size):
        ret = None
        if obj in self._assigned_prefixes:
            ret = self._assigned_prefixes[obj]
        else:
            ret = self._prefix_tree.get_prefix(prefix_size)
            self._assigned_prefixes[obj] = ret
        return ret

    def get_control_address(self, obj):
        ret = None
        if not self.control_prefix:
            raise RuntimeError("No control prefix given")
        try:
            ret = self._assigned_addresses[obj]
        except KeyError:
            ret = next(self._ctrl_prefix_it)
            self._assigned_addresses[obj] = ret
        return ret

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    #@inline_task
    def __create__(self):
        # XXX code from Channel.__create__, until Events are properly implemented.
        # Iterate on channels for allocate IP addresses
        task = EmptyTask()
        for group in self.groups:
            for channel in group.iter_by_type_str('channel'):
                interfaces = sorted(channel.interfaces, key = lambda x : x.device_name)
                if not interfaces:
                    continue

                min_prefix_size = math.ceil(math.log(len(channel.interfaces), 2))
                prefix_size = min(self.max_prefix_size,
                        self.PrefixClass.MAX_PREFIX_SIZE - min_prefix_size)
                prefix = self.get_prefix(channel, prefix_size)

                it = prefix.get_iterator()

                for interface in interfaces:
                    ip = next(it)
                    interface.set(self.ATTR_PREFIX, prefix_size)
                    #XXX Why do we need to create that async task?
                    #XXX Probably because the PendingValue is not created
                    #XXX in the main thread
                    @async_task
                    async def set_ip(interface, ip):
                        await interface.async_set(self.ATTR_ADDRESS, self.PrefixClass.ntoa(ip.ip_address))
                    task = task | set_ip(interface, ip)

        return task

    __delete__ = None

class Ipv6Assignment(IpAssignment):
    PrefixClass = Inet6Prefix
    ATTR_ADDRESS = 'ip6_address'
    ATTR_PREFIX  = 'ip6_prefix'

class Ipv4Assignment(IpAssignment):
    PrefixClass = Inet4Prefix
    ATTR_ADDRESS = 'ip4_address'
    ATTR_PREFIX  = 'ip4_prefix'
