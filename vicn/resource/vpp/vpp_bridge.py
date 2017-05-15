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

from netmodel.model.type                import Integer
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.attribute                import Reference
from vicn.core.exception                import ResourceNotFound
from vicn.core.requirement              import Requirement
from vicn.core.resource_mgr             import wait_resource_task
from vicn.core.resource                 import Resource
from vicn.core.task                     import task, BashTask, EmptyTask
from vicn.resource.channel              import Channel
from vicn.resource.linux.application    import LinuxApplication
from vicn.resource.linux.sym_veth_pair  import SymVethPair
from vicn.resource.linux.sym_veth_pair  import SymVethPair
from vicn.resource.node                 import Node
from vicn.resource.vpp.dpdk_device      import DpdkDevice
from vicn.resource.vpp.interface        import VPPInterface
from vicn.resource.vpp.vpp              import VPP

CMD_ADD_INTERFACE_TO_BR = ('vppctl set interface l2 bridge '
        '{interface.device_name} {br_domain}')

class VPPBridge(Channel, LinuxApplication):
    """
    Resource: VPPBridge

    VPPBridge instantiate a vpp resource and set it as a vpp bridge.

    This resource requires to be run within a LxcContainer which will have VPP.
    Every interface in the lxc_container (i.e., the ones contained in
    self.node.interfaces) will be added to the vpp bridge. To connect other vpp
    node to the bridge, the corresponding dpdkdevice must be added as an
    interface to the channel.
    """

    # The vpp bridge _USES_ a VPP forwarder on the node
    node = Attribute(Node, mandatory=True,
                     description = 'Node on which vpp is running',
                     requirements = [Requirement('vpp')])

    connected_nodes = Attribute(Node, multiplicity = Multiplicity.OneToMany,
                     description = 'List of nodes to connect to the bridge')

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._vpp_interfaces = list()

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __subresources__ (self):
        # We don't need any reference to the list of SymVethPair because each
        # side of a veth will be included in the node.interfaces list
        self._veths = [SymVethPair(node1 = self.node, node2 = node,
                owner = self) for node in self.connected_nodes]

        return Resource.__concurrent__(*self._veths)

    @task
    def __initialize__ (self):
        # Add the veth side on the connected_nodes to the set of interfaces of
        # the channel
        self.interfaces.extend([veth.side2 for veth in self._veths])

    @task
    def __get__(self):
        # Forces creation
        raise ResourceNotFound

    # Nothing to do
    __delete__ = None

    def __create__(self):
        manager = self._state.manager

        # Create a VPPInterface for each interface in the node. These will be
        # the interfaces we will connect to the vpp bridge process
        vpp_interfaces = list()
        for interface in self.node.interfaces:
            # FIXME harcoded value
            if interface.device_name == 'eth0':
                continue

            vpp_interface = VPPInterface(vpp = self.node.vpp,
                    parent = interface,
                    ip4_address = Reference(interface, 'ip4_address'),
                    device_name = 'host-' + interface.device_name)
            vpp_interfaces.append(vpp_interface)
            manager.commit_resource(vpp_interface)

        tasks = EmptyTask()

        for vpp_interface in vpp_interfaces:
            tasks = tasks > (wait_resource_task(vpp_interface) >
                    self._add_interface(vpp_interface,0))

        return wait_resource_task(self.node.vpp) > tasks

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _add_interface(self, interface, br_domain):
        return BashTask(self.node, CMD_ADD_INTERFACE_TO_BR,
                {'interface': interface, 'br_domain': br_domain})

    def _del_interface(self,  interface, br_domain):
        raise NotImplementedError('Interface removal not supported')

