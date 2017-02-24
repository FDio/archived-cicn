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

import asyncio

from netmodel.model.type                import String, Integer, Bool
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import Resource
from vicn.core.task                     import BashTask, task, inline_task
from vicn.resource.lxd.lxc_container    import LxcContainer
from vicn.resource.node                 import Node
from vicn.resource.linux.file           import TextFile
from vicn.resource.vpp.dpdk_device      import DpdkDevice
from vicn.resource.vpp.scripts          import FN_VPP_DPDK_SCRIPT
from vicn.resource.vpp.scripts          import TPL_VPP_DPDK_DAEMON_SCRIPT
from vicn.resource.vpp.vpp_commands     import CMD_VPP_DISABLE, CMD_VPP_STOP
from vicn.resource.vpp.vpp_commands     import CMD_VPP_START
from vicn.resource.vpp.vpp_commands     import CMD_VPP_ENABLE_PLUGIN

#------------------------------------------------------------------------------
# VPP forwarder
#------------------------------------------------------------------------------

CMD_GET  = 'killall -0 vpp_main'
CMD_DISABLE_IP_FORWARD = 'sysctl -w net.ipv4.ip_forward=0'

class VPP(Resource):
    """
    Todo:
     - make VPP an application with package install
     - vpp should be a service (hence a singleton) for which we override the
     start and stop commands
    """

    #__package_names__ = ['vpp', 'vpp-dbg', 'vpp-dpdk-dev']

    plugins = Attribute(String,
            multiplicity = Multiplicity.OneToMany)
    node = Attribute(Node,
            multiplicity = Multiplicity.OneToOne,
            reverse_name = 'vpp')
    numa_node = Attribute(Integer, 
            description = 'Numa node on which vpp will run')
    core = Attribute(Integer, 
            description = 'Core belonging the numa node on which vpp will run')
    enable_worker = Attribute(Bool, 
            description = 'Enable one worker for packet processing',
            default = False)

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.vppctl_lock = asyncio.Lock()

        self.dpdk_setup_file = None
        if isinstance(self.node, LxcContainer):
            if not 'vpp' in self.node.profiles:
                self.node.profiles.append('vpp')

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        return ['BaseNetDevice']

    def __get__(self):
        return BashTask(self.node, CMD_GET)

    def __subresources__(self):
        self.dpdk_setup_file = TextFile(node = self.node, 
                filename = FN_VPP_DPDK_SCRIPT, 
                overwrite = True)
        return self.dpdk_setup_file

    def __create__(self):
        socket_mem = dict()
        numa_mgr = self.node.node_with_kernel.numa_mgr

        for interface in self.node.interfaces:
            if isinstance(interface, DpdkDevice):
                # Assign as numa node the first numa node specified in a
                # physical card (if any). If multiple nics connected to
                # different numa nodes are assigned to this vpp memory access
                # will be inefficient for the nics sitting in the other numa
                # node.
                socket_mem[interface.numa_node] = interface.socket_mem

        for iface in self.interfaces:
            if isinstance(iface.parent, DpdkDevice) and \
                    not iface.parent.numa_node is None:
                self.numa_node = iface.parent.numa_node
                break
        if self.numa_node is None or self.core is None:
            self.numa_node, self.core = \
                    numa_mgr.get_numa_core(numa_node = self.numa_node)

        dpdk_list = list()

        # On numa architecture socket-mem requires to set the amount of memory
        # to be reserved on each numa node
        socket_mem_str = 'socket-mem '
        for numa in range (0,numa_mgr.get_number_of_numa()):
            if numa in socket_mem:
                socket_mem_str = socket_mem_str + str(socket_mem[numa])
            else:
                socket_mem_str = socket_mem_str + '0'

            if numa < numa_mgr.get_number_of_numa()-1:
                socket_mem_str = socket_mem_str + ','

        dpdk_list.append(socket_mem_str)

        for interface in self.node.interfaces:
            if isinstance(interface, DpdkDevice):
                dpdk_list.append('dev ' + interface.pci_address)

        # Add the core on which running vpp and the dpdk parameters
        setup = TPL_VPP_DPDK_DAEMON_SCRIPT + 'cpu {'
            
        setup = setup + ''' \n  main-core ''' + str(self.core)

        if self.enable_worker:
            self.numa_node, cpu_worker =numa_mgr.get_numa_core(self.numa_node)
            setup = setup + '''\n  corelist-workers ''' + str(cpu_worker)

        setup = setup + '''\n}\n\n  dpdk { '''
        
        for dpdk_dev in dpdk_list:
            setup = setup + ''' \n  ''' + dpdk_dev

        setup = setup + '\n}'


        if any([isinstance(interface,DpdkDevice) for interface in self.node.interfaces]):
                self.dpdk_setup_file.content = setup
        else:
                self.dpdk_setup_file.content = TPL_VPP_DPDK_DAEMON_SCRIPT

        lock = self.node.node_with_kernel.vpp_host.vppstart_lock

        vpp_disable = BashTask(self.node, CMD_VPP_DISABLE, lock = lock)
        vpp_stop = BashTask(self.node, CMD_VPP_STOP, lock = lock)
        enable_ip_forward = BashTask(self.node, CMD_DISABLE_IP_FORWARD)
        start_vpp = BashTask(self.node, CMD_VPP_START, lock = lock)

        return ((vpp_disable > vpp_stop) | enable_ip_forward) > start_vpp

    def __delete__(self):
        return BashTask(self.node, CMD_VPP_STOP)

    def _add_plugins(self, plugin):
        return BashTask(self.node, CMD_VPP_ENABLE_PLUGIN, {'plugin': plugin})

    def _set_plugins(self):
        cmd = None
        for plugin in self.plugins:
            cmd = cmd > BashTask(self.node, CMD_VPP_ENABLE_PLUGIN, 
                    {'plugin' : plugin})
        return cmd

    def _remove_plugins(self, plugin):
        raise NotImplementedError

    @inline_task
    def _get_plugins(self):
        return {'plugins' : []}
