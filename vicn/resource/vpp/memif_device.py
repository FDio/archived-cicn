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
from netmodel.model.type                import Bool, String, Integer
from netmodel.model.type                import Inet4Address, Inet6Address
from vicn.core.attribute                import Attribute
from vicn.resource.interface            import Interface
from vicn.resource.lxd.lxc_container    import CMD_MOUNT_FOLDER
from vicn.resource.linux.folder         import Folder
from vicn.core.task                     import inline_task, BashTask
from vicn.core.task                     import inherit_parent
from vicn.core.exception                import ResourceNotFound
from vicn.core.address_mgr              import AddressManager

class MemifDevice(Interface):
    """
    Resource: Memory interface device

    A MemifDevice is device build on top of a the memory interface provided by vpp.
    It uses a unix socket to connect the two vpp-s (one master and one slave).
    The unix socket must be shared between the two vpp-s.
    """
    path_unix_socket = Attribute(String,
            mandatory = True,
            description = 'Path to the shared folder holding the unix socket')
    socket_name = Attribute(String,
            mandatory = True,
            description = 'Path to the shared folder holding the unix socket')
    folder_host = Attribute(Folder,
            mandatory = True,
            description = 'Folder in the host to be mounted in the container ih path_unix_socket')
    master= Attribute(Bool,
            description = 'True if this interface is connected to the master vpp',
            default = False)
    # We need to automatically assign a mac address to the memif so that we can
    # recreate it after vpp reboots thanks to a config file or bash script.
    # Just reading the actual value would not work since we need to use an
    # external script and this mac address would thus not be prevent in the list
    # of executed commands.
    mac_address = Attribute(String, description = 'Mac address of the device',
            default = lambda self: AddressManager().get_mac(self))
    ip4_address = Attribute(Inet4Address, description = "Device's IP address")
    ip6_address = Attribute(Inet6Address, description = "Device's IP address")
    device_name = Attribute(String)
    key = Attribute(Integer)

    __key__ = Key(folder_host)

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    @inherit_parent
    def __create__(self):
        return BashTask(self.node.node_with_kernel, CMD_MOUNT_FOLDER, {
                'container': self.node,
                'device-name': self.socket_name,
                'host_path': self.folder_host.foldername,
                'container_path': self.path_unix_socket}, output=True,
                lock=self.node.vpp.memif_lock)

