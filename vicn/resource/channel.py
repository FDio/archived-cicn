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

from netmodel.model.type    import String
from vicn.core.resource     import Resource
from vicn.core.attribute    import Attribute
from vicn.core.task         import EmptyTask
from vicn.resource.ip_assignment    import Ipv6Assignment, Ipv4Assignment

from math import log, ceil

class Channel(Resource):
    """
    Resource: Channel
    """

    #--------------------------------------------------------------------------
    # Public API
    #--------------------------------------------------------------------------

    def __after_init__(self):
        return ("IpAssignment",)

    def get_remote_name(self, name):
        if len(self._interfaces) != 2:
            return None
        return next(x for x in self._interfaces if x.get_name() != name)

    def get_sortable_name(self):
        """
        This method is used to sort channel during IP assignment. This is
        necessary to get the same IP configuration on the same experiment.
        """
        ret = "{:03}".format(len(self.interfaces))
        ret = ret + ''.join(sorted(map(lambda x : x.node.name, self.interfaces)))
        return ret

    def __create__(self):
        interfaces = sorted(self.interfaces, key = lambda x : x.device_name)
        if interfaces:
            #IPv6
            central6 = self._state.manager.by_type(Ipv6Assignment)[0]
            prefix6_size = min(64, 128 - ceil(log(len(self.interfaces), 2)))
            prefix6 = iter(central6.get_prefix(self, prefix6_size))

            #IPv4
            central4 = self._state.manager.by_type(Ipv4Assignment)[0]
            prefix4_size = 32 - ceil(log(len(self.interfaces), 2))
            prefix4 = iter(central4.get_prefix(self, prefix4_size))

            for interface in interfaces:
                try:
                    interface.ip4_address = next(prefix4)
                except StopIteration as e:
                    import pdb; pdb.set_trace()
                interface.ip6_address = next(prefix6)
                interface.ip6_prefix = prefix6_size

        return EmptyTask()
