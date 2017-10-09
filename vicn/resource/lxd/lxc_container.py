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

import logging
import shlex
import time
import requests

# Suppress logging from pylxd dependency on ws4py
# (this needs to be included before pylxd)
from ws4py import configure_logger
configure_logger(level=logging.ERROR)
import pylxd

from netmodel.model.type            import String, Integer, Bool, Self
from vicn.core.attribute            import Attribute, Reference, Multiplicity
from vicn.core.commands             import ReturnValue
from vicn.core.exception            import ResourceNotFound
from vicn.core.requirement          import Requirement
from vicn.core.resource_mgr         import wait_resource_task
from vicn.core.task                 import task, inline_task, BashTask, EmptyTask
from vicn.core.task                 import inherit_parent
from vicn.resource.linux.net_device import NetDevice
from vicn.resource.node             import Node
from vicn.resource.vpp.scripts      import APPARMOR_VPP_PROFILE
from vicn.resource.lxd.lxc_image    import LxcImage
from vicn.resource.lxd.lxd_profile  import LXD_PROFILE_DEFAULT_IFNAME

log = logging.getLogger(__name__)

# Default remote server (pull mode only)
DEFAULT_SOURCE_URL      = 'https://cloud-images.ubuntu.com/releases/'

# Default protocol used to download images (lxd or simplestreams)
DEFAULT_SOURCE_PROTOCOL = 'simplestreams'

# Commands used to interact with LXD (in addition to pylxd bindings)
CMD_GET_PID='lxc info {container.name} | grep Pid | cut -d " " -f 2'

CMD_UNSET_IP6_FWD = 'sysctl -w net.ipv6.conf.all.forwarding=0'
CMD_SET_IP6_FWD = 'sysctl -w net.ipv6.conf.all.forwarding=1'
CMD_GET_IP6_FWD = 'sysctl -n net.ipv6.conf.all.forwarding'

CMD_NETWORK_DHCP='dhclient {container.management_interface.device_name}'

CMD_MOUNT_FOLDER='''
lxc config device add {container.name} {device-name} disk source={host_path} path={container_path}
sleep 1
'''

# Type: ContainerName
# https://github.com/lxc/lxd/issues/1431
# [...] all container names must be a valid hostname under the most
#restrictive definition of this, that is, maximum 63 characters, may not contain
#dots, may not start by a digit or dash, may not end by a dash and must be made
#entirely of letters, digits or hyphens.
# XXX better have a allowed property
ContainerName = String.restrict(max_size = 63, ascii = True,
        forbidden = ('/', ',', ':', '_'))

class LxcContainer(Node):
    """
    Resource: LxcContainer

    Todo:
      - Remove VPP dependency
      - The bridge is not strictly needed, but we currently have no automated
      way to determine whether we need it or not
      - The management interface should be added by VICN, not part of the
      resource, and its name should be determined automatically.
    """

    architecture = Attribute(String, description = 'Architecture',
            default = 'x86_64')
    container_name = Attribute(ContainerName,
            description = 'Name of the container',
            default = Reference(Self, 'name'))
    ephemeral = Attribute(Bool, description = 'Ephemeral container flag',
            default = False)
    node = Attribute(Node,
            description = 'Node on which the container is running',
            mandatory = True,
            requirements = [
                # We need the hypervisor setup to be able to check for the
                # container; more generally, all dependencies
                Requirement('lxd_hypervisor'), # not null
                # The bridge is not strictly needed, but we currently have
                # no automated way to determine whether we need it or not
                Requirement('bridge'),
                # A DNS server is required to provide internet connectivity to
                # the containers
                # Requirement('dns_server'),
            ])
    profiles = Attribute(String, multiplicity = Multiplicity.OneToMany,
            default = ['vicn'])
    image = Attribute(LxcImage, description = 'image', default = None)
    is_image = Attribute(Bool, default = False)
    pid = Attribute(Integer, description = 'PID of the container', ro = True)
    ip6_forwarding = Attribute(Bool, default=True)

    #--------------------------------------------------------------------------
    # Constructor / Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._container = None

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inherit_parent
    @inline_task
    def __initialize__(self):
        """
        We need to intanciate VPPHost before container creation.
        """
        self.node_with_kernel = Reference(self, 'node')

        # We automatically add the management/monitoring interface
        self._management_interface = NetDevice(node = self,
                owner = self,
                monitored = False,
                device_name = LXD_PROFILE_DEFAULT_IFNAME)
        self._state.manager.commit_resource(self._management_interface)

        for iface in self.interfaces:
            if iface.get_type() == "dpdkdevice":
                self.node.vpp_host.dpdk_devices.append(iface.pci_address)

    @inherit_parent
    @task
    def __get__(self):
        client = self.node.lxd_hypervisor.client
        try:
            self._container = client.containers.get(self.name)
        except pylxd.exceptions.NotFound:
            raise ResourceNotFound

    @inherit_parent
    def __create__(self):
        """
        Make sure vpp_host is instanciated before starting the container.
        """
        wait_vpp_host = EmptyTask()
        if 'vpp' in self.profiles:
            wait_vpp_host = wait_resource_task(self.node.vpp_host)
        create = self._create_container()
        start = self.__method_start__()
        #XXX Should be an option on the netdevice
        dhcp_interface = BashTask(self, CMD_NETWORK_DHCP, {'container':self})
        return (wait_vpp_host > (create > start)) > dhcp_interface

    @task
    def _create_container(self):
        container = self._get_container_description()
        log.debug('Container description: {}'.format(container))
        print('Container description: {}'.format(container))
        client = self.node.lxd_hypervisor.client

        self._container = client.containers.create(container, wait=True)

    def _get_container_description(self):
        # Base configuration
        container = {
            'name'          : self.container_name,
            'architecture'  : self.architecture,
            'ephemeral'     : self.ephemeral,
            'profiles'      : self.profiles,
            'config'        : {},
            'devices'       : {},
        }

        # DEVICES

        # FIXME Container profile support is provided by setting changes into
        # configuration (currently only vpp profile is supported)
        for profile in self.profiles:
            if profile == 'vpp':
                # Set the new apparmor profile. This will be created in VPP
                # application
                # Mount hugetlbfs in the container.
                container['config']['raw.lxc'] = APPARMOR_VPP_PROFILE
                container['config']['security.privileged'] = 'true'

                for device in self.node.vpp_host.uio_devices:
                    container['devices'][device] = {
                        'path' : '/dev/{}'.format(device),
                        'type' : 'unix-char' }

        # SOURCE

        image_names = [alias['name'] for alias in self.node.lxd_hypervisor.aliases]
        image_exists = self.image.image is not None and self.image.image in image_names

        if image_exists:
            container['source'] = {
                'type'      : 'image',
                'mode'      : 'local',
                'alias'     : self.image.image,
            }
        else:
            container['source'] = {
                'type'      : 'image',
                'mode'      : 'pull',
                'server'    : DEFAULT_SOURCE_URL,
                'protocol'  : DEFAULT_SOURCE_PROTOCOL,
                'alias'     : self.dist,
            }

        log.info('Creating container: {}'.format(container))
        return container

    @task
    def __delete__(self):
        log.info("Delete container {}".format(self.container_name))
        self.node.lxd_hypervisor.client.containers.remove(self.name)

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def _get_pid(self):
        """
        Attribute: pid (getter)
        """
        return BashTask(self.node, CMD_GET_PID, {'container': self},
                parse = lambda rv: {'pid': rv.stdout.strip()})

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    @task
    def __method_start__(self):
        """
        Method: Start the container
        """
        self._container.start(wait = True)
        time.sleep(1)

    @task
    def __method_stop__(self):
        """
        Method: Stop the container
        """
        self._container.stop(wait = True)

    @task
    def __method_to_image__(self):
        """
        Returns:
            Image metadata as returned by LXD REST API.
        """
        publish_description = {
            "public": True,
            "properties": {
                "os": "Ubuntu",
                "architecture": "x86_64",
                "description": "Image generated from container {}".format(
                        self.container_name),
            },
            "source": {
                "type": "container",  # One of "container" or "snapshot"
                "name": 'image-{}'.format(self.container_name),
            }
        }
        return self.node.lxd_hypervisor.publish_image(publish_description)

    #--------------------------------------------------------------------------
    # Node API
    #--------------------------------------------------------------------------

    def execute(self, command, output = False, as_root = False):
        """
        Executes a command on the node

        Params:
            output (bool) : Flag determining whether the method should return
                the output value.
            as_root (bool) : Flag telling whether the command should be
                executed as root.

        Returns:
            ReturnValue containing exit code, and eventually stdout and stderr.

        Raises
            Exception in case of error

        The node exposes an interface allowing command execution through LXD.
        We don't currently use an eventually available  SSH connection.
        """

        if not self._container:
            log.error("Executing command on uninitialized container {} {}".format(self, command))
            import os; os._exit(1)

        if 'vppctl_wrapper' in command:
            vpp_log = '{}/vpp-{}.sh'.format(self._state.manager._base, self.name)
            with open(vpp_log, 'a') as f:
                print("lxc exec {} -- {}".format(self.name, command), file=f)

        # XXX Workaround: pylxd 2.2.3 buggy  (w/ lxd 2.14) ?
        # But this workaround is broken with lxd 2.15 and pylxd 2.2.4 works
        # lxc exec freezes
        #return self.node.execute('lxc exec {} -- {}'.format(self.name, command),
        #        output = output, as_root = as_root)

        print("lxc exec {} -- {}".format(self.name, command))
        while True:
            try:
                ret = self._container.execute(shlex.split(command))
                break
            except pylxd.exceptions.NotFound:
                print("=====> pylxd not found during {}".format(command))
                time.sleep(1)
            except pylxd.exceptions.ClientConnectionFailed:
                print("=====> pylxd connection failed during {}".format(command))
                time.sleep(1)
            except requests.exceptions.SSLError:
                print("=====> ssl error during {}".format(command))
                time.sleep(1)


        # NOTE: pylxd documents the return value as a tuple, while it is in
        # fact a ContainerExecuteResult object
        if not hasattr(ret, "exit_code"):
            log.error("LXD return value does not have an exit code. "
                    "Try installing pylxd>=2.2.2 with pip3")
            import sys; sys.exit(1)

        args = (ret.exit_code,)
        if output:
            args += (ret.stdout, ret.stderr)
        return ReturnValue(*args)

    def _get_ip6_forwarding(self):
        return BashTask(self, CMD_GET_IP6_FWD,
            parse = lambda rv: {'ip6_forwarding' : rv.stdout == "1"})

    def _set_ip6_forwarding(self):
        cmd = CMD_SET_IP6_FWD if self.ip6_forwarding else CMD_UNSET_IP6_FWD
        return BashTask(self, cmd)
