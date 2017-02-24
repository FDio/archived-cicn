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
import logging

from netmodel.model.type            import Integer, String
from vicn.core.attribute            import Attribute, Reference
from vicn.core.exception            import ResourceNotFound
from vicn.core.state                import ResourceState, AttributeState
from vicn.core.task                 import inline_task, async_task, run_task
from vicn.core.task                 import get_attributes_task, BashTask
from vicn.resource.channel          import Channel
from vicn.resource.interface        import Interface
from vicn.resource.linux.net_device import NonTapBaseNetDevice
from vicn.resource.node             import Node

# FIXME remove VPP specific code
from vicn.resource.vpp.interface    import VPPInterface

log = logging.getLogger(__name__)

CMD_DELETE_IF_EXISTS='ip link show {interface.device_name} && ' \
                     'ip link delete {interface.device_name} || true'

CMD_CREATE='''
# Create veth pair in the host node
ip link add name {tmp_src} type veth peer name {tmp_dst}
ip link set dev {tmp_src} netns {pid[0]} name {interface.src.device_name}
ip link set dev {tmp_dst} netns {pid[1]} name {interface.dst.device_name}
'''
CMD_UP='''
ip link set dev {interface.device_name} up
'''

class Link(Channel):
    """
    Resource: Link

    Implements a virtual wired link between containers. It is a VethPair, both
    sides of which sit inside a different container.

    Because of this, the resource only supports passing source and destination
    containers, and not interfaces. It also explains the relative complexity of
    the current implementation.
    """

    src = Attribute(Interface, description = 'Source interface')
    dst = Attribute(Interface, description = 'Destination interface')

    capacity = Attribute(Integer, description = 'Link capacity (Mb/s)')
    delay = Attribute(String, description = 'Link propagation delay')

    src_node = Attribute(Node, description = 'Source node', 
            mandatory = True)
    dst_node = Attribute(Node, description = 'Destination node',
            mandatory = True)

    def __init__(self, *args, **kwargs):
        assert 'src' not in kwargs and 'dst' not in kwargs
        assert 'src_node' in kwargs and 'dst_node' in kwargs
        super().__init__(*args, **kwargs)

    @inline_task
    def __initialize__(self):
    
        # We create two managed net devices that are pre-setup
        # but the resource manager has to take over for IP addresses etc.
        # Being done in initialize, those attributes won't be considered as
        # dependencies and will thus not block the resource state machine.
        self.src = NonTapBaseNetDevice(node = self.src_node, 
                device_name = self.dst_node.name,
                channel = self,
                capacity = self.capacity,
                owner = self.owner)
        self.dst = NonTapBaseNetDevice(node = self.dst_node, 
                device_name = self.src_node.name,
                channel = self,
                capacity = self.capacity,
                owner = self.owner)
        self.dst.remote = self.src 
        self.src.remote = self.dst

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    async def _commit(self):
        manager = self._state.manager

        # We mark the src and dst interfaces created because we are pre-setting
        # them up in __create__ using a VethPair
        # We go through both INITIALIZED and CREATED stats to raise the proper
        # events and satisfy any eventual wait_* command.
        await manager._set_resource_state(self.src, ResourceState.INITIALIZED)
        await manager._set_resource_state(self.dst, ResourceState.INITIALIZED)
        await manager._set_resource_state(self.src, ResourceState.CREATED)
        await manager._set_resource_state(self.dst, ResourceState.CREATED)

        # We mark the attribute clean so that it is not updated
        await manager._set_attribute_state(self, 'src', AttributeState.CLEAN)
        await manager._set_attribute_state(self, 'dst', AttributeState.CLEAN)

        manager.commit_resource(self.src)
        manager.commit_resource(self.dst)

        # Disable rp_filtering
        # self.src.rp_filter = False
        # self.dst.rp_filter = False

        #XXX VPP
        if hasattr(self.src_node, 'vpp') and not self.src_node.vpp is None:
            vpp_src = VPPInterface(parent = self.src,
                    vpp = self.src_node.vpp,
                    ip_address = Reference(self.src, 'ip_address'),
                    device_name = 'vpp' + self.src.device_name)
            manager.commit_resource(vpp_src)

        if hasattr(self.dst_node, 'vpp') and not self.dst_node.vpp is None:
            vpp_dst = VPPInterface(parent = self.dst,
                    vpp = self.dst_node.vpp,
                    ip_address = Reference(self.dst, 'ip_address'),
                    device_name = 'vpp' + self.dst.device_name)
            manager.commit_resource(vpp_dst)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @async_task
    async def __get__(self):
        manager = self._state.manager

        try:
            await run_task(self.src.__get__(), manager)
            await run_task(self.dst.__get__(), manager)
        except ResourceNotFound:
            # This is raised if any of the two side of the VethPair is missing
            raise ResourceNotFound

        # We always need to commit the two endpoints so that their attributes
        # are correctly updated
        await self._commit()
            
    def __create__(self):
        assert self.src_node.get_type() == 'lxccontainer'
        assert self.dst_node.get_type() == 'lxccontainer'

        src_host = self.src_node.node
        dst_host = self.dst_node.node

        assert src_host == dst_host
        host = src_host

        # Sometimes a down interface persists on one side
        delif_src = BashTask(self.src_node, CMD_DELETE_IF_EXISTS, 
                {'interface': self.src})
        delif_dst = BashTask(self.dst_node, CMD_DELETE_IF_EXISTS, 
                {'interface': self.dst})

        pid_src = get_attributes_task(self.src_node, ['pid'])
        pid_dst = get_attributes_task(self.dst_node, ['pid'])

        tmp_src = 'tmp-veth-' + ''.join(random.choice(string.ascii_uppercase +
                    string.digits) for _ in range(5))
        tmp_dst = 'tmp-veth-' + ''.join(random.choice(string.ascii_uppercase + 
                    string.digits) for _ in range(5))

        create = BashTask(host, CMD_CREATE, {'interface': self,
                'tmp_src': tmp_src, 'tmp_dst': tmp_dst})

        up_src = BashTask(self.src_node, CMD_UP, {'interface': self.src})
        up_dst = BashTask(self.dst_node, CMD_UP, {'interface': self.dst})

        @async_task
        async def set_state():
            # We always need to commit the two endpoints so that their attributes
            # are correctly updated
            await self._commit()

        delif = delif_src | delif_dst
        up    = up_src | up_dst
        pid   = pid_src | pid_dst
        return ((delif > (pid @ create)) > up) > set_state()

