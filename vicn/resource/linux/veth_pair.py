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

from netmodel.model.key                 import Key
from netmodel.model.type                import String, Bool
from vicn.core.attribute                import Attribute, Reference
from vicn.core.task                     import BashTask, inline_task, get_attributes_task
from vicn.resource.linux.net_device     import NetDevice, NetDeviceName
from vicn.resource.node                 import Node
from vicn.resource.symmetric_channel    import SymmetricChannel

CMD_CREATE='''
# Create veth pair in the host node
ip link add name {self.device_name} type veth peer name {self.peer_device_name}
'''

DEPRECATED_CMD_UP='''
# The host interface will always be up...
ip link set dev {interface.host.device_name} up

# Move interface into container and rename it
ip link set dev {tmp_name} netns {pid} name {interface.device_name}
ip link set dev {interface.device_name} up
'''

# Forward declaration
class VethPair(SymmetricChannel):
    pass

class VethNetDevice(NetDevice):
    parent = Attribute(VethPair, mandatory = True, ro = True)

    __get__ = None
    __create__ = None
    __delete__ = None

class VethPair(SymmetricChannel):
    # Mimics NetDevice for using its __get__ and __delete__ functions
    node = Attribute(Node)
    device_name = Attribute(NetDeviceName)
    peer_device_name = Attribute(NetDeviceName)
    capacity = Attribute(String)
    src = Attribute(ro = True, mandatory = False)
    dst = Attribute(ro = True, mandatory = False)
    auto_commit = Attribute(Bool, description = 'Auto commit interfaces')

    __key1__ = Key(node, device_name)
    __key2__ = Key(node, peer_device_name)

    @inline_task
    def _commit(self):
        if self.auto_commit:
            manager = self._state.manager

            manager.commit_resource(self.src)
            manager.commit_resource(self.dst)

    def __initialize__(self):
        # XXX owner prevents the resource to be committed
        self.src = VethNetDevice(node = self.node,
                parent = self,
                device_name = self.device_name,
                channel = self,
                capacity = Reference(self, 'capacity'),
                owner = self)
        self.dst = VethNetDevice(node = self.node,
                parent = self,
                device_name = self.peer_device_name,
                channel = self,
                capacity = Reference(self, 'capacity'),
                owner = self)

    def __create__(self):
        veth = BashTask(self.node, CMD_CREATE, {'self': self})
        return (veth > super().__create__()) > self._commit()

    def __get__(self):
        return NetDevice.__get__(self) > self._commit()

    def __delete__(self):
        return NetDevice.__delete__(self)
