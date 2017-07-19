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

from netmodel.model.type            import String
from vicn.core.attribute            import Attribute, Multiplicity
from vicn.core.exception            import ResourceNotFound
from vicn.core.requirement          import Requirement
from vicn.core.task                 import BashTask, task, EmptyTask
from vicn.core.task                 import inherit_parent
from vicn.resource.linux.application import LinuxApplication
from vicn.resource.linux.file       import TextFile
from vicn.resource.node             import Node
from vicn.resource.vpp.scripts      import FN_APPARMOR_DPDK_SCRIPT
from vicn.resource.vpp.scripts      import TPL_APPARMOR_DPDK_SCRIPT
from vicn.resource.vpp.scripts      import FN_VPP_DPDK_SCRIPT
from vicn.resource.vpp.scripts      import TPL_VPP_DPDK_DAEMON_SCRIPT
from vicn.resource.vpp.vpp_commands import CMD_VPP_DISABLE
from vicn.resource.vpp.vpp_commands import CMD_VPP_STOP_SERVICE

CMD_INSERT_MODULES = 'modprobe uio && modprobe igb_uio'
CMD_APP_ARMOR_RELOAD = '''
# Force apparmor to reload profiles to include the new profile
/etc/init.d/apparmor reload
'''
CMD_SYSCTL_HUGEPAGES = 'sysctl -w vm.nr_hugepages={nb_hp}'
DEFAULT_NB_HUGEPAGES = 1024
CMD_GREP_UIO_DEV = 'ls /dev | grep uio'
CMD_CREATE_UIO_DEVICES = "dpdk-devbind --bind=igb_uio {pci_address}"

class VPPHost(LinuxApplication):
    """
    Resource: VPPHost

    Only used for container deployment

    Packages required on the host
     - vpp : sysctl configuration
     - vpp-dpdk-dkms : kernel modules

    Host must be configured to let vpp to work into container:
     - install new apparmor profile (to let the container to read
       hugepages info in /sys/kernel/mm/hugepages)
     - set hugepages into the host
    """

    node =  Attribute(Node,
            description = 'Node on which the application is installed',
            mandatory = True,
            multiplicity = Multiplicity.OneToOne,
            reverse_name = 'vpp_host',
            reverse_description = 'Setup for hosting vpp containers',
            reverse_auto = True,
            requirements = [Requirement('numa_mgr')])
    uio_devices = Attribute(String,
            description = 'uio devices on the node',
            multiplicity = Multiplicity.OneToMany,
            ro = True)
    dpdk_devices = Attribute(String,
            description = 'Dpdk devices on the node',
            multiplicity = Multiplicity.OneToMany)

    __package_names__ = ['vpp-dpdk-dkms', 'vpp-dpdk-dev']

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.vppstart_lock = asyncio.Lock()

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inherit_parent
    def __subresources__(self):
        app_armor_file = TextFile(node = self.node,
                filename = FN_APPARMOR_DPDK_SCRIPT,
                content = TPL_APPARMOR_DPDK_SCRIPT,
                overwrite = True)
        startup_conf = TextFile(node = self.node,
                filename = FN_VPP_DPDK_SCRIPT,
                content = TPL_VPP_DPDK_DAEMON_SCRIPT,
                overwrite = True)
        return app_armor_file | startup_conf

    @inherit_parent
    @task
    def __get__(self):
        """
        This method always assumes the resource does not exist, since it is not
        an issue to perform the modprobe call everytime.
        """
        raise ResourceNotFound

    @inherit_parent
    def __create__(self):
        modules = BashTask(self.node, CMD_INSERT_MODULES)
        app_armor_reload = BashTask(self.node, CMD_APP_ARMOR_RELOAD)
        sysctl_hugepages = BashTask(self.node, CMD_SYSCTL_HUGEPAGES,
                {'nb_hp': DEFAULT_NB_HUGEPAGES})

        # Hook
        # The following is needed to create uio devices in /dev. They are
        # required to let vpp to use dpdk (or other compatibles) nics. From a
        # container, vpp cannot create those devices, therefore we need to
        # create them in the host and then mount them on each container running
        # vpp (and using a physical nic)
        stop_vpp = BashTask(self.node, CMD_VPP_STOP_SERVICE + " || true")
        disable_vpp = BashTask(self.node, CMD_VPP_DISABLE + " || true")
        disable_vpp = stop_vpp > disable_vpp

        create_uio = EmptyTask()
        for device in self.dpdk_devices:
            create_uio = create_uio > BashTask(self.node,
                    CMD_CREATE_UIO_DEVICES, {'pci_address' : device})

        return ((modules | app_armor_reload) | sysctl_hugepages) > \
            (disable_vpp > create_uio)

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def _get_uio_devices(self):
        def parse(rv):
            return rv.stdout.splitlines()
        return BashTask(self.node, CMD_GREP_UIO_DEV, parse = parse)
