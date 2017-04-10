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
ip link set dev {tmp_src} netns {pid[0]} name {interface._src.device_name}
ip link set dev {tmp_dst} netns {pid[1]} name {interface._dst.device_name}
'''

#Changing namespace brings interfaces down
CMD_CREATE_BR_TO_LXC='''
ip link add name {tmp_src} type veth peer name {tmp_dst}
ip link set dev {tmp_src} netns {pid} name {interface.device_name}
ip link set dev {tmp_dst} up
ovs-vsctl add-port {host.bridge.device_name} {tmp_dst}'''

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

    capacity = Attribute(Integer, description = 'Link capacity (Mb/s)')
    delay = Attribute(String, description = 'Link propagation delay')

    src_node = Attribute(Node, description = 'Source node',
            mandatory = True)
    dst_node = Attribute(Node, description = 'Destination node',
            mandatory = True)

    def __init__(self, *args, **kwargs):
        assert 'src_node' in kwargs and 'dst_node' in kwargs
        self._src = None
        self._dst = None
        super().__init__(*args, **kwargs)

    @inline_task
    def __initialize__(self):
        # We create two managed net devices that are pre-setup
        # but the resource manager has to take over for IP addresses etc.
        # Being done in initialize, those attributes won't be considered as
        # dependencies and will thus not block the resource state machine.
        self._src = NonTapBaseNetDevice(node = self.src_node,
                device_name = self.dst_node.name,
                channel = self,
                capacity = self.capacity,
                owner = self)
        self._dst = NonTapBaseNetDevice(node = self.dst_node,
                device_name = self.src_node.name,
                channel = self,
                capacity = self.capacity,
                owner = self)
        self._dst.remote = self._src
        self._src.remote = self._dst

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    async def _commit(self):
        manager = self._state.manager

        manager.commit_resource(self._src)
        manager.commit_resource(self._dst)

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

    def __get__(self):
        return (self._src.__get__() | self._dst.__get__()) > async_task(self._commit)()

    def __create__(self):
        assert self.src_node.get_type() == 'lxccontainer'
        assert self.dst_node.get_type() == 'lxccontainer'

        src_host = self.src_node.node
        dst_host = self.dst_node.node
        #assert src_host == dst_host

        # Sometimes a down interface persists on one side
        delif_src = BashTask(self.src_node, CMD_DELETE_IF_EXISTS,
                {'interface': self._src})
        delif_dst = BashTask(self.dst_node, CMD_DELETE_IF_EXISTS,
                {'interface': self._dst})

        pid_src = get_attributes_task(self.src_node, ['pid'])
        pid_dst = get_attributes_task(self.dst_node, ['pid'])

        tmp_src = 'tmp-veth-' + ''.join(random.choice(string.ascii_uppercase +
                    string.digits) for _ in range(5))
        tmp_dst = 'tmp-veth-' + ''.join(random.choice(string.ascii_uppercase +
                    string.digits) for _ in range(5))
        pid   = pid_src | pid_dst

        if src_host == dst_host:
            host = src_host
            create = BashTask(host, CMD_CREATE, {'interface': self,
                'tmp_src': tmp_src, 'tmp_dst': tmp_dst})
            up_src = BashTask(self.src_node, CMD_UP, {'interface': self._src})
            up_dst = BashTask(self.dst_node, CMD_UP, {'interface': self._dst})
            up    = up_src | up_dst
            delif = delif_src | delif_dst
            return ((delif > (pid @ create)) > up) > async_task(self._commit)()
        else:
            create = BashTask(src_host, CMD_CREATE_BR_TO_LXC, {'interface': self._src,
                'tmp_src': tmp_src, 'tmp_dst': tmp_dst, 'host' : src_host})
            create2 = BashTask(dst_host, CMD_CREATE_BR_TO_LXC, {'interface': self._dst,
                'tmp_src': tmp_dst, 'tmp_dst': tmp_src, 'host' : dst_host})
            up_src = BashTask(self.src_node, CMD_UP, {'interface': self._src})
            up_dst = BashTask(self.dst_node, CMD_UP, {'interface': self._dst})
            return (((pid_src @ create) | (pid_dst @ create2)) > (up_src | up_dst))  > async_task(self._commit)()


    def __delete__(self):
        return self._src.__delete__() | self._dst.__delete__()
