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

from netmodel.model.type            import Integer, String, Bool
from vicn.core.resource             import Resource
from vicn.core.attribute            import Attribute, Multiplicity
from vicn.core.exception            import ResourceNotFound
from vicn.core.task                 import inline_task, BashTask, task
from vicn.core.task                 import EmptyTask
from vicn.resource.interface        import Interface
from vicn.resource.linux.net_device import NonTapBaseNetDevice
from vicn.resource.vpp.vpp          import VPP
from vicn.resource.vpp.vpp_commands import CMD_VPP_CREATE_IFACE
from vicn.resource.vpp.vpp_commands import CMD_VPP_SET_IP, CMD_VPP_SET_UP

class VPPInterface(Resource):
    """
    Resource: VPPInterface

    An interface representation in VPP
    """

    vpp = Attribute(VPP, 
            description = 'Forwarder to which this interface belong to',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne,
            key = True,
            reverse_name = 'interfaces') 
    parent = Attribute(Interface, description = 'parent', 
            mandatory = True, reverse_name = 'vppinterface') 
    ip_address = Attribute(String)
    prefix_len = Attribute(Integer, default = 31)
    up = Attribute(Bool, description = 'Interface up/down status')
    monitored = Attribute(Bool, default = True)

    device_name = Attribute(String)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        """
        We need CentralIP to get the parent interface IP address
        """
        return ['CentralIP']

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    def __create__(self):
        # We must control what is the type of the parent netDevice (currently
        # supported only veths, physical nics are coming)
        create_task = EmptyTask()

        # We must let the routing algorithm know that the parent interface
        # belongs to vpp
        self.parent.has_vpp_child = True

        self.ip_address = self.parent.ip_address
        self.up = True

        if isinstance(self.parent,NonTapBaseNetDevice):
            # Remove ip address in the parent device, it must only be set in
            # the vpp interface otherwise vpp and the linux kernel will reply
            # to non-icn request (e.g., ARP replies, port ureachable etc)

            self.device_name = 'host-' + self.parent.device_name
            create_task = BashTask(self.vpp.node, CMD_VPP_CREATE_IFACE,
                    {'vpp_interface': self},
                    lock = self.vpp.vppctl_lock)

            self.parent.set('ip_address', None)
            self.parent.set('offload', False)
            self.parent.remote.set('offload', False)

        elif self.parent.get_type() == 'dpdkdevice':
            self.device_name = self.parent.device_name
        else :
            # Currently assume naively that everything else will be a physical
            # NIC for VPP
            #
            # Before initialization, we need to make sure that the parent
            # interface is down (vpp will control the nic)
            self.device_name = 'host-' + self.parent.device_name
            self.parent.set('up', False)

        return create_task

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def _set_ip_address(self):
        if self.ip_address: 
            return BashTask(self.vpp.node, CMD_VPP_SET_IP, {'netdevice': self},
                    lock = self.vpp.vppctl_lock)

    def _set_up(self):
        return BashTask(self.vpp.node, CMD_VPP_SET_UP, {'netdevice': self},
                    lock = self.vpp.vppctl_lock)

    @task
    def _get_up(self):
        return {'up' : False}

    @task
    def _get_ip_address(self):
        return {'ip_address' : None}
