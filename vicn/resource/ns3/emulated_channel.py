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

import functools
import logging
import random

from netmodel.model.key                 import Key
from netmodel.model.type                import Integer
from netmodel.util.socket		import check_port
from vicn.core.address_mgr              import AddressManager
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.exception                import ResourceNotFound
from vicn.core.requirement              import Requirement
from vicn.core.resource                 import BaseResource
from vicn.core.resource_mgr             import wait_resources
from vicn.core.task                     import inline_task, async_task, task
from vicn.core.task                     import BashTask, run_task
from vicn.core.task                     import inherit_parent
from vicn.resource.channel              import Channel
from vicn.resource.linux.application    import LinuxApplication
from vicn.resource.linux.net_device     import NetDevice
from vicn.resource.linux.tap_device     import TapDevice
from vicn.resource.linux.veth_pair_lxc  import VethPairLxc
from vicn.resource.lxd.lxc_container    import LxcContainer
from vicn.resource.node                 import Node

log = logging.getLogger(__name__)

class EmulatedChannel(Channel, LinuxApplication):
    """EmulatedChannel resource

    This resources serves as a base class for wireless channels emulated by
    means of ns3 simulation.

    Attributes:
        ap (Reference[node]): Reference to the AP node
        stations (Reference[node]): Reference to the list of stations.
        control_port (int): Port used to communicate with the management
            interface of the simulation.

    Implementation notes:
      - Both AP and stations are allocated a separate VLAN to isolate broadcast
        traffic and prevent loops on the bridge.
      - We also need that all interfaces related to ap and stations are created
        before we run the commandline (currently, dynamically adding/removing
        AP and stations is not supported by the emulator). This is made
        possible thanks to the key=True parameter, which makes sure the
        attributes are processed before the __create__ is called.

    Todo:
      - Retrieve the process PID to kill it during __delete__
    """

    __resource_type__ = BaseResource

    ap = Attribute(Node, description = 'AP')
    stations = Attribute(Node, description = 'List of stations',
            multiplicity = Multiplicity.OneToMany)
    control_port = Attribute(Integer,
            description = 'Control port for the simulation')
    nb_base_stations = Attribute(Integer, description='Number of nodes emulated by the AP',
            default=1)

    __key__ = Key(ap, stations)

    # Overloaded attributes
    node = Attribute(requirements = [
            Requirement('bridge')
        ])

    #--------------------------------------------------------------------------
    # Constructor
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # AP (resp. stations) interfaces (for external connectivity) and
        # tap_devices (for connection to emulator)
        self._ap_if = None
        self._ap_tap = None
        self._sta_ifs = dict()
        self._sta_taps = dict()

        # Device names to be attached to the bridge (differs according to the
        # node type, Physical or LxcContainer, and eventually None for an
        # unmanaged stations)
        self._ap_bridged = None
        self._sta_bridged = dict()

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inherit_parent
    def __initialize__(self):
        return self.__set_ap() > self.__set_stations()

    @inherit_parent
    @inline_task
    def __get__(self):
        raise ResourceNotFound


    @inherit_parent
    def __create__(self):
        # NOTE: http://stackoverflow.com/questions/21141352/python-subprocess-
        #       calling-a-script-which-runs-a-background-process-hanging
        # The output of the background scripts is still going to the same file
        # descriptor as the child script, thus the parent script waits for it
        # to finish.
        def get_cmdline(channel):
            return '(' + channel.__app_name__ + ' ' + \
                  channel._get_cmdline_params() + '>/dev/null 2>&1) &'
        return BashTask(self.node, functools.partial(get_cmdline, self))

    @inherit_parent
    def __delete__(self):
        raise NotImplementedError

    #--------------------------------------------------------------------------
    # Attribute handlers
    #--------------------------------------------------------------------------

    @async_task
    async def __set_ap(self):
        ap = self.ap
        if ap is None:
            log.info('Ignored setting ap to None...')
            return

        # Add a WiFi interface for the AP...
        interfaces = list()
        if isinstance(ap, LxcContainer):
            # Ideally, We need to create a VethPairLxc for each station
            # This should be monitored for the total channel bw
            host = NetDevice(node = ap.node,
                device_name='vhh-' + ap.name + '-' + self.name,
                monitored = False,
                managed = False)
            self._ap_if = VethPairLxc(node = self.ap,
                    name          = 'vh-' + ap.name + '-' + self.name,
                    device_name   = 'vh-' + ap.name + '-' + self.name,
                    host          = host,
                    owner = self)
            self._ap_bridged = self._ap_if.host
        else:
            raise NotImplementedError
        self._state.manager.commit_resource(self._ap_if)

        interfaces.append(self._ap_if)

        # Add a tap interface for the AP...
        self._ap_tap = TapDevice(node = self.node,
                owner = self,
                device_name = 'tap-' + ap.name + '-' + self.name,
                up = True,
                promiscuous = True,
                monitored = False)
        self._state.manager.commit_resource(self._ap_tap)
        interfaces.append(self._ap_tap)

        # Wait for interfaces to be setup
        await wait_resources(interfaces)

        # NOTE: only set channel after the resource is created or it might
        # create loops which, at this time, are not handled
        self._ap_if.set('channel', self)

        # Add interfaces to bridge
        vlan = AddressManager().get('vlan', self, tag='ap')

        # AS the container has created the VethPairLxc already without Vlan, we
        # need to delete and recreate it
        task = self.node.bridge._remove_interface(self._ap_bridged)
        await run_task(task, self._state.manager)
        task = self.node.bridge._add_interface(self._ap_bridged, vlan = vlan)
        await run_task(task, self._state.manager)

        task = self.node.bridge._remove_interface(self._ap_tap)
        await run_task(task, self._state.manager)
        task = self.node.bridge._add_interface(self._ap_tap, vlan = vlan)
        await run_task(task, self._state.manager)

    @inline_task
    def __get_ap(self):
        return {'ap': None}

    @inline_task
    def __get_stations(self):
        return {'stations': list()}

    @async_task
    async def __set_stations(self):
        for station in self.stations:
            await self._add_station(station)

    def __add_stations(self, stations):
        raise NotImplementedError

    @inline_task
    def __remove_stations(self, station):
        raise NotImplementedError

