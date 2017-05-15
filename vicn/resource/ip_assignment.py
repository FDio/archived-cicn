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

from vicn.core.resource                 import Resource
from netmodel.model.type                import String
from vicn.core.attribute                import Attribute
from vicn.resource.ip.prefix_tree       import Inet6Prefix, PrefixTree, Inet4Prefix
from vicn.core.task                     import inline_task
from vicn.core.exception                import ResourceNotFound

class IpAssignment(Resource):
    prefix = Attribute(String, mandatory=True)
    control_prefix = Attribute(String, description="prefix for control plane")

    PrefixClass = None

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._prefix = self.PrefixClass(self.prefix)
        self._prefix_tree = PrefixTree(self._prefix)
        self._assigned_prefixes = {}
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

class Ipv6Assignment(IpAssignment):
    PrefixClass = Inet6Prefix


class Ipv4Assignment(IpAssignment):
    PrefixClass = Inet4Prefix
