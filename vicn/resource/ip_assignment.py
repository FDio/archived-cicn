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
from netmodel.model.type                import String, Bool
from vicn.core.attribute                import Attribute
from vicn.resource.ip.prefix_tree       import Inet6Prefix, PrefixTree, Inet4Prefix
from netmodel.model.type                import Inet4Address, Inet6Address
from vicn.core.task                     import inline_task, async_task, EmptyTask
from vicn.core.task                     import inherit_parent
from vicn.core.exception                import ResourceNotFound

log = logging.getLogger(__name__)

class IpAssignment(Resource):
    prefix = Attribute(String, mandatory=True)
    control_prefix = Attribute(String, description="prefix for control plane")
    max_prefix_len = Attribute(String,
            description="Maximum assigned prefix size for a link")
    disjoint_addresses = Attribute(Bool, default=False)

    PrefixClass = None

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._prefix = self.PrefixClass(self.prefix)
        self._prefix_tree = PrefixTree(self._prefix)
        self._assigned_prefixes = {}
        if not self.max_prefix_len:
            self.max_prefix_len = self.PrefixClass.MAX_PREFIX_SIZE
        if self.control_prefix:
            self._ctrl_prefix = self.PrefixClass(self.control_prefix)
            self._ctrl_prefix_it = iter(self._ctrl_prefix)
            next(self._ctrl_prefix_it) #Removes internet address
            self._assigned_addresses = {}

    def get_prefix(self, obj, prefix_len):
        ret = None
        if obj in self._assigned_prefixes:
            ret = self._assigned_prefixes[obj]
        else:
            ret = self._prefix_tree.get_prefix(prefix_len)
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

    @inherit_parent
    @inline_task
    def __get__(self):
        raise ResourceNotFound

    @inherit_parent
    #@inline_task
    def __create__(self):
        # XXX code from Channel.__create__, until Events are properly implemented.
        # Iterate on channels for allocate IP addresses
        task = EmptyTask()
        for group in self.groups:
            for channel in group.iter_by_type_str('channel'):
                if channel.has_type("emulatedchannel"):
                    interfaces = [channel._ap_if]
                    interfaces.extend(channel._sta_ifs.values())
                else:
                    interfaces = sorted(channel.interfaces, key = lambda x : x.device_name)
                    if not interfaces:
                        continue

                if self.PrefixClass is Inet6Prefix:
                    # 0 is forbidden
                    num_required_ip = len(interfaces) + 1
                elif channel.has_type("emulatedchannel"): #EmulatedChannel + IPv4
                    num_required_ip = len(interfaces) + 2 + channel.nb_base_stations #Internet address + bcast
                else:
                    num_required_ip = len(interfaces)
                min_prefix_len = math.ceil(math.log(num_required_ip, 2))

                prefix_len = min(self.max_prefix_len,
                        self.PrefixClass.MAX_PREFIX_SIZE - min_prefix_len)

                #XXX lte-emulator expects /24
                if channel.has_type("emulatedltechannel") and self.PrefixClass is Inet4Prefix:
                    prefix_len = 24

                # Retrieve a prefix for the whole channel
                prefix = self.get_prefix(channel, prefix_len)

                # by default iterate on /32 or /128, unless we require to
                # iterate on less specific
                it_prefix_len = self.PrefixClass.MAX_PREFIX_SIZE

                if self.disjoint_addresses and prefix_len+min_prefix_len <= self.PrefixClass.MAX_PREFIX_SIZE:
                    it_prefix_len = min_prefix_len + prefix_len

                # Use an iterator to assign IPs from the prefix to the
                # interfaces
                it = prefix.get_iterator(it_prefix_len)
                # XXX MACCHA it is a prefix

                if channel.has_type("emulatedchannel"):
                    #Internet address
                    next(it)

                for interface in interfaces:
                    if_prefix = next(it)
                    #XXX We cannot assign prefix::0 as a valid address to an interface.
                    #XXX For each interface with an ip6 address belonging to prefix,
                    #XXX linux add prefix::0 to the local routing table. Therefore prefix::0 cannot be
                    #XXX the address of a non local interface
                    ip = if_prefix.ip_address
                    if ip == prefix.first_prefix_address() and self.PrefixClass is Inet6Prefix:
                        if if_prefix.prefix_len < if_prefix.MAX_PREFIX_SIZE:
                            if_prefix.ip_address = ip+1
                        else:
                            if_prefix = next(it)

                    if_prefix.prefix_len = prefix_len
                    if self.PrefixClass is Inet6Prefix:
                        address = Inet6Address(if_prefix.ip_address, prefix_len)
                    else:
                        address = Inet4Address(if_prefix.ip_address, prefix_len)
                    @async_task
                    async def set_ip(interface, address):
                        await interface.async_set(self.ATTR_ADDRESS, address)
                    task = task | set_ip(interface, address)
        return task

class Ipv6Assignment(IpAssignment):
    PrefixClass = Inet6Prefix
    ATTR_ADDRESS  = 'ip6_address'

class Ipv4Assignment(IpAssignment):
    PrefixClass = Inet4Prefix
    ATTR_ADDRESS  = 'ip4_address'
