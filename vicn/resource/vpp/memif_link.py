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
import asyncio

from netmodel.model.type                import Integer, String
from netmodel.model.key                 import Key
from vicn.core.attribute                import Attribute, Reference
from vicn.core.exception                import ResourceNotFound
from vicn.core.state                    import ResourceState, AttributeState
from vicn.core.task                     import inline_task, async_task, run_task
from vicn.core.task                     import get_attributes_task, BashTask
from vicn.core.task                     import inherit_parent
from vicn.resource.channel              import Channel
from vicn.resource.interface            import Interface
from vicn.resource.linux.net_device     import NonTapBaseNetDevice
from vicn.resource.node                 import Node
from vicn.resource.lxd.lxc_container    import CMD_MOUNT_FOLDER, LxcContainer
from vicn.resource.vpp.memif_device     import MemifDevice
from vicn.resource.linux.folder         import Folder

# FIXME remove VPP specific code
from vicn.resource.vpp.interface    import VPPInterface

log = logging.getLogger(__name__)

CONTAINER_SOCKET_PATH='/root/{}'
SHARED_FOLDER_PATH='/tmp/{}'

class MemifLink(Channel):
    """
    Resource: MemifLink

    Implements a virtual wired link between containers. It is made
    with a pair of MemifDevice which are created in the two containers.

    Because of this, the resource only supports passing source and destination
    containers, and not interfaces. It also explains the relative complexity of
    the current implementation.
    """

    src_node = Attribute(LxcContainer, description = 'Source node',
            mandatory = True)
    dst_node = Attribute(LxcContainer, description = 'Destination node',
            mandatory = True)
    folder = Attribute(Folder, description = 'Shared folder holding the socket used by the MemifDevices')
    key = Attribute(Integer)

    __key__ = Key(src_node, dst_node)

    def __init__(self, *args, **kwargs):
        assert 'src_node' in kwargs and 'dst_node' in kwargs
        self._src = None
        self._dst = None
        self._folder = None
        super().__init__(*args, **kwargs)

    @inherit_parent
    def __subresources__(self):
        assert self.src_node.node_with_kernel == self.dst_node.node_with_kernel

        host = self.src_node.node_with_kernel
        # We create two managed net devices that are pre-setup
        # but the resource manager has to take over for IP addresses etc.
        # Being done in initialize, those attributes won't be considered as
        # dependencies and will thus not block the resource state machine.

        socket_name = 'socket-' + ''.join(random.choice(string.ascii_uppercase
                            + string.digits) for _ in range(5)) +'.file'

        _folder = Folder(node = host,
                foldername = SHARED_FOLDER_PATH.format(self.src_node.name + '-' + self.dst_node.name),
                permission = 777)

        self.key = random.randint(0, 2**64)

        self._src = MemifDevice(node = self.src_node,
                channel = self,
                owner = self,
                path_unix_socket = CONTAINER_SOCKET_PATH.format(self.src_node.name + '-'
                            + self.dst_node.name) + '/',
                folder_host = _folder,
                socket_name = socket_name,
                device_name = 'memif-'+self.dst_node.name,
                key = Reference(self, 'key'),
                master = False)
        self._dst = MemifDevice(node = self.dst_node,
                channel = self,
                owner = self,
                path_unix_socket = CONTAINER_SOCKET_PATH.format(self.src_node.name + '-'
                            + self.dst_node.name) + '/',
                socket_name = socket_name,
                folder_host = _folder,
                device_name = 'memif-'+self.src_node.name,
                key = Reference(self, 'key'),
                master = True)
        self._dst.remote = self._src
        self._src.remote = self._dst

        return _folder | (self._src | self._dst)


    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    async def _commit(self):
        manager = self._state.manager

        if hasattr(self.src_node, 'vpp') and not self.src_node.vpp is None:
            vpp_src = VPPInterface(parent = self._src,
                    vpp = self.src_node.vpp,
                    ip4_address = Reference(self._src, 'ip4_address'),
                    ip6_address = Reference(self._src, 'ip6_address'))
            manager.commit_resource(vpp_src)

        if hasattr(self.dst_node, 'vpp') and not self.dst_node.vpp is None:
            vpp_dst = VPPInterface(parent = self._dst,
                    vpp = self.dst_node.vpp,
                    ip4_address = Reference(self._dst, 'ip4_address'),
                    ip6_address = Reference(self._dst, 'ip6_address'))
            manager.commit_resource(vpp_dst)

    @inherit_parent
    def __get__(self):
        return async_task(self._commit)()
