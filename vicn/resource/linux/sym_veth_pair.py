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

import random
import string

from netmodel.model.type            import Integer
from vicn.core.attribute            import Attribute
from vicn.core.exception            import ResourceNotFound
from vicn.core.resource             import Resource
from vicn.core.state                import ResourceState, AttributeState
from vicn.core.task                 import BashTask, get_attributes_task
from vicn.core.task                 import async_task, task, inline_task
from vicn.core.task                 import run_task
from vicn.resource.interface        import Interface
from vicn.resource.node             import Node
from vicn.resource.linux.net_device import NonTapBaseNetDevice
from vicn.resource.linux.link       import CMD_DELETE_IF_EXISTS
from vicn.resource.linux.link       import CMD_UP

CMD_CREATE='''
# Create veth pair in the host node
ip link add name {tmp_side1} type veth peer name {tmp_side2}
ip link set dev {tmp_side1} netns {pid[0]} name {side1_device_name}
ip link set dev {tmp_side2} netns {pid[1]} name {side2_device_name}
'''

class SymVethPair(Resource):
    """
    Resource: SymVethPair

    This resource is used in VPPBridge. The main difference with the Link
    resource is that is it not a channel.
    """

    node1 = Attribute(Node, 
            description = 'Node on which one side of the veth will sit',
            mandatory = True)
    node2 = Attribute(Node, 
            description = 'Node on which the other side of the veth will sit',
            mandatory = True)
    capacity = Attribute(Integer, 
            description = 'Capacity of the veth pair (Mb/s)')
    side1 = Attribute(Interface, description = 'Source interface')
    side2 = Attribute(Interface, description = 'Destination interface')

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    async def _commit(self):
        # see link.py for explanations
        manager = self._state.manager
        await manager._set_resource_state(self.side1, 
                ResourceState.INITIALIZED)
        await manager._set_resource_state(self.side2, 
                ResourceState.INITIALIZED)
        await manager._set_resource_state(self.side1, ResourceState.CREATED)
        await manager._set_resource_state(self.side2, ResourceState.CREATED)
        await manager._set_attribute_state(self, 'side1', AttributeState.CLEAN)
        await manager._set_attribute_state(self, 'side2', AttributeState.CLEAN)
        manager.commit_resource(self.side1)
        manager.commit_resource(self.side2)        

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __initialize__(self):
        self.side1 = NonTapBaseNetDevice(node = self.node1,
                device_name = self.node2.name,
                capacity = self.capacity,
                owner = self.owner)
        self.side2 = NonTapBaseNetDevice(node = self.node2,
                device_name = self.node1.name,
                capacity = self.capacity,
                owner = self.owner)
        self.side1.remote = self.side2
        self.side2.remote = self.side1

    @async_task
    async def __get__(self):
        manager = self._state.manager

        try:
            await run_task(self.side1.__get__(), manager)
            await run_task(self.side2.__get__(), manager)
        except ResourceNotFound:
            raise ResourceNotFound

        await self._commit()

    def __create__(self):
        assert self.node1.get_type() == 'lxccontainer'
        assert self.node2.get_type() == 'lxccontainer'
        
        node1_host = self.node1.node
        node2_host = self.node2.node
        
        assert node1_host == node2_host
        host = node1_host
        
        # Sometimes a down interface persists on one side
        delif_side1 = BashTask(self.node1, CMD_DELETE_IF_EXISTS, 
                {'interface': self.side1})
        delif_side2 = BashTask(self.node2, CMD_DELETE_IF_EXISTS, 
                {'interface': self.side2})
     
        pid_node1 = get_attributes_task(self.node1, ['pid'])
        pid_node2 = get_attributes_task(self.node2, ['pid'])
        
        tmp_side1 = 'tmp-veth-' + ''.join(random.choice(
                    string.ascii_uppercase + string.digits) for _ in range(5))
        tmp_side2 = 'tmp-veth-' + ''.join(random.choice(
                    string.ascii_uppercase + string.digits) for _ in range(5))
        
        create = BashTask(host, CMD_CREATE, 
                {'side1_device_name': self.side1.device_name,
                'side2_device_name': self.side2.device_name, 
                'tmp_side1': tmp_side1, 'tmp_side2': tmp_side2})
     
        up_side1 = BashTask(self.node1, CMD_UP, {'interface': self.side1})
        up_side2 = BashTask(self.node2, CMD_UP, {'interface': self.side2})
     
        @async_task
        async def set_state():
            await self._commit()
        
        delif = delif_side1 | delif_side2
        up    = up_side1 | up_side2
        pid   = pid_node1 | pid_node2
        return ((delif > (pid @ create)) > up) > set_state()
    
    def __delete__(self):
        raise NotImplementedError
