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

import pyparsing as pp

from netmodel.model.key             import Key
from netmodel.model.type            import Integer, String, Bool
from netmodel.model.type            import Inet4Address, Inet6Address
from vicn.core.resource             import Resource
from vicn.core.attribute            import Attribute, Multiplicity
from vicn.core.exception            import ResourceNotFound
from vicn.core.task                 import inline_task, BashTask, task
from vicn.core.task                 import inherit_parent
from vicn.core.task                 import EmptyTask
from vicn.resource.interface        import Interface
from vicn.resource.linux.net_device import NonTapBaseNetDevice
from vicn.resource.vpp.vpp          import VPP
from vicn.resource.vpp.vpp_commands import CMD_VPP_CREATE_IFACE, CMD_VPP_CREATE_MEMIFACE
from vicn.resource.vpp.vpp_commands import CMD_VPP_SET_IP, CMD_VPP_SET_UP
from vicn.resource.vpp.memif_device import MemifDevice

VPP_vers = (17, 7)

GREP_MEMIF_INFO = 'vppctl_wrapper show memif | grep interface --no-group-separator -A 1'

def parse_memif(rv, vppinterface):
    kw_interface = pp.CaselessKeyword('interface')
    kw_key = pp.CaselessKeyword('key')
    kw_remote_name =  pp.CaselessKeyword('remote-name')
    kw_remote_if =  pp.CaselessKeyword('remote-interface')
    kw_id = pp.CaselessKeyword('id')
    kw_mode = pp.CaselessKeyword('mode')
    kw_ethernet = pp.CaselessKeyword('ethernet')
    kw_file = pp.CaselessKeyword('file')
    kw_listener = pp.CaselessKeyword('listener')
    kw_connfd = pp.CaselessKeyword('conn-fd')
    kw_intfd = pp.CaselessKeyword('int-fd')
    kw_ringsize = pp.CaselessKeyword('ring-size')
    kw_numc2srings = pp.CaselessKeyword('num-c2s-rings')
    kw_nums2crings = pp.CaselessKeyword('num-s2c-rings')
    kw_buffersize = pp.CaselessKeyword('buffer_size')

    r_path = ' *(/[a-zA-Z0-9_\-]*)*\.[a-zA-Z0-9_\-]*'
    r_id = ' *-+[0-9]*'

    r_remote_name = '"[A-Z]+ [0-9\.]+-[a-z]+"'
    r_remote_if   = '"[a-zA-Z0-9_\-]+"'

    if VPP_vers >= (17, 7):
        single = kw_interface.suppress() + pp.Word(pp.alphanums).setResultsName('interface') + \
                 pp.Optional(kw_remote_name.suppress() + pp.Regex(r_remote_name).setResultsName('remote_name') + \
                 kw_remote_if.suppress() + pp.Regex(r_remote_if).setResultsName('remote_if')) + \
                 kw_id.suppress() + pp.Word(pp.alphanums).setResultsName('key') + \
                 kw_mode.suppress() + kw_ethernet.suppress() + \
                 kw_file.suppress() + pp.Regex(r_path).setResultsName('path')  # + \
                 # kw_listener.suppress() + pp.Word(pp.alphanums).setResultsName('listener') + \
                 # kw_connfd.suppress() + pp.Regex(r_id).setResultsName('conn-fd') + \
                 # kw_intfd.suppress() + pp.Regex(r_id).setResultsName('int-fd') + \
                 # kw_ringsize.suppress() + pp.Word(pp.nums).setResultsName('ring-size') + \
                 # kw_numc2srings.suppress() + pp.Word(pp.nums).setResultsName('num-c2s-rings') + \
                 # kw_nums2crings.suppress() + pp.Word(pp.nums).setResultsName('num-s2c-rings') + \
                 # kw_buffersize.suppress() + pp.Word(pp.nums).setResultsName('buffer-size')
    else:
        single = kw_interface.suppress() + pp.Word(pp.alphanums).setResultsName('interface') + \
                 kw_key.suppress() + pp.Word(pp.alphanums).setResultsName('key') + \
                 kw_file.suppress() + pp.Regex(r_path).setResultsName('path')  # + \
                 # kw_listener.suppress() + pp.Word(pp.alphanums).setResultsName('listener') + \
                 # kw_connfd.suppress() + pp.Regex(r_id).setResultsName('conn-fd') + \
                 # kw_intfd.suppress() + pp.Regex(r_id).setResultsName('int-fd') + \
                 # kw_ringsize.suppress() + pp.Word(pp.nums).setResultsName('ring-size') + \
                 # kw_numc2srings.suppress() + pp.Word(pp.nums).setResultsName('num-c2s-rings') + \
                 # kw_nums2crings.suppress() + pp.Word(pp.nums).setResultsName('num-s2c-rings') + \
                 # kw_buffersize.suppress() + pp.Word(pp.nums).setResultsName('buffer-size')

    multiple = pp.OneOrMore(pp.Group(single))

    try:
         results = multiple.parseString(rv.stdout)
    except Exception as e:
         import pdb; pdb.set_trace()

    for interface in results:
        if interface['path'] == vppinterface.parent.path_unix_socket + vppinterface.parent.socket_name:
            vppinterface.device_name = interface['interface']
            vppinterface.parent.device_name = interface['interface']

    return vppinterface.device_name


class VPPInterface(Resource):
    """
    Resource: VPPInterface

    An interface representation in VPP
    """

    vpp = Attribute(VPP,
            description = 'Forwarder to which this interface belong to',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne,
            reverse_name = 'interfaces')
    parent = Attribute(Interface, description = 'parent',
            mandatory = True, reverse_name = 'vppinterface')
    ip4_address = Attribute(Inet4Address)
    ip6_address = Attribute(Inet6Address)
    up = Attribute(Bool, description = 'Interface up/down status')
    monitored = Attribute(Bool, default = True)

    device_name = Attribute(String)

    __key__ = Key(vpp, Resource.name)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        """
        We need CentralIP to get the parent interface IP address
        """
        return ['CentralIP', 'IpAssignment']

    @inherit_parent
    @inline_task
    def __get__(self):
        raise ResourceNotFound

    @inherit_parent
    def __create__(self):
        # We must control what is the type of the parent netDevice (currently
        # supported only veths, physical nics are coming)
        create_task = EmptyTask()

        # We must let the routing algorithm know that the parent interface
        # belongs to vpp
        self.parent.has_vpp_child = True
        self.up = True

        self.ip4_address = self.parent.ip4_address
        self.ip6_address = self.parent.ip6_address

        if isinstance(self.parent, MemifDevice):
            self.device_name = self.parent.device_name

            #TODO: add output parsing to get the interface name
            if VPP_vers >= (17, 7):
                create_task = BashTask(self.vpp.node, CMD_VPP_CREATE_MEMIFACE, {
                    'key_label': 'id',
                    'key': self.parent.key,
                    'vpp_interface': self,
                    'master_slave': 'master' if self.parent.master else 'slave'},
                    lock = self.vpp.vppctl_lock)
            else:
                create_task = BashTask(self.vpp.node, CMD_VPP_CREATE_MEMIFACE, {
                    'key_label': 'key',
                    'key': hex(self.parent.key),
                    'vpp_interface': self,
                    'master_slave': 'master' if self.parent.master else 'slave'},
                    lock = self.vpp.vppctl_lock)
            fill_name =  BashTask(self.vpp.node, GREP_MEMIF_INFO,
                                 parse = (lambda x, y=self : parse_memif(x, y)),
                                 lock = self.vpp.vppctl_lock)

            create_task = create_task > fill_name

        elif isinstance(self.parent,NonTapBaseNetDevice):
            # Remove ip address in the parent device, it must only be set in
            # the vpp interface otherwise vpp and the linux kernel will reply
            # to non-icn request (e.g., ARP replies, port ureachable etc)
            self.parent.ip4_address = None
            self.parent.ip6_address = None

            self.device_name = 'host-' + self.parent.device_name
            create_task = BashTask(self.vpp.node, CMD_VPP_CREATE_IFACE,
                    {'vpp_interface': self},
                    lock = self.vpp.vppctl_lock)

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

    def _set_ip4_address(self):
        if self.ip4_address:
            return BashTask(self.vpp.node, CMD_VPP_SET_IP, {
                    'device_name': self.device_name,
                    'ip_address': str(self.ip4_address),
                    'prefix_len': self.ip4_address.prefix_len},
                    lock = self.vpp.vppctl_lock)

    def _set_ip6_address(self):
        if self.ip6_address:
            return BashTask(self.vpp.node, CMD_VPP_SET_IP, {
                    'device_name': self.device_name,
                    'ip_address': str(self.ip6_address),
                    'prefix_len': self.ip6_address.prefix_len},
                    lock = self.vpp.vppctl_lock)

    def _set_up(self):
        state = 'up' if self.up else 'down'
        return BashTask(self.vpp.node, CMD_VPP_SET_UP, {
                'netdevice': self,
                'state': state},
                lock = self.vpp.vppctl_lock)

    @task
    def _get_up(self):
        return {'up' : False}

    @task
    def _get_ip4_address(self):
        return {'ip4_address' : None}

    @task
    def _get_ip6_address(self):
        return {'ip6_address' : None}
