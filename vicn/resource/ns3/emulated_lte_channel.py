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

from vicn.core.address_mgr              import AddressManager
from vicn.core.resource_mgr             import wait_resources
from vicn.core.task                     import run_task
from vicn.resource.ns3.emulated_channel import EmulatedChannel
from vicn.resource.linux.net_device     import NetDevice

DEFAULT_FADING_ENABLED = True
DEFAULT_TW_BUFFER = 800000
DEFAULT_NETMASK = 24

class EmulatedLteChannel(EmulatedChannel):
    """
    Resource: EmulatedLteChannel

    This resource uses ns3 based emulation to emulate a lte subnet with one pgw,
    one enode B, and multiple UEs.

    NOTE:
    This model needs to be extended with LTE specific features like "network
    resource" in the future. Currently it works in same way as wifi emulator
    with no lte specific features.

    Attributes:
        ap (Reference[node]): Reference to the AP/pgw node
        stations (Reference[node]): Reference to the list of UE.
        control_port (int): Port used to communicate with the management
            interface of the simulation.
    """

    __package_names__ = ['lte-emulator']
    __app_name__ = 'lte_emulator'

    #---------------------------------------------------------------------------
    # Attribute handlers
    #---------------------------------------------------------------------------

    async def _add_station(self, station):
        from vicn.resource.lxd.lxc_container    import LxcContainer
        from vicn.resource.linux.veth_pair      import VethPair
        from vicn.resource.linux.tap_device     import TapChannel

        interfaces = list()
        # ... and each station
        if not station.managed:
            sta_if = None
        else:
            if isinstance(station, LxcContainer):
                host = NetDevice(node = station.node,
                    device_name='vhh-' + station.name + '-' + self.name,
                    managed = False)
                sta_if = VethPair(node = station,
                        name          = 'vh-' + station.name + '-' + self.name,
                        device_name   = 'vh-' + station.name + '-' + self.name, 
                        host          = host,
                        owner = self)
                bridged_sta = sta_if.host
            else:
                raise NotImplementedError

        if sta_if:
            self._sta_ifs[station] = sta_if
            self._sta_bridged[station] = bridged_sta
            interfaces.append(sta_if)
            self._state.manager.commit_resource(sta_if)

        sta_tap = TapChannel(node = self.node,
                owner = self,
                device_name = 'tap-' + station.name + '-' + self.name,
                up = True,
                promiscuous = True,
                station_name = station.name,
                channel_name = self.name)
        self._sta_taps[station] = sta_tap
        interfaces.append(sta_tap)
        self._state.manager.commit_resource(sta_tap)

        # Wait for interfaces to be setup
        await wait_resources(interfaces)

        # Add interfaces to bridge
        # One vlan per station is needed to avoid broadcast loops
        vlan = AddressManager().get('vlan', sta_tap)
        if sta_if:
            sta_if.set('channel', self)

            task = self.node.bridge._remove_interface(bridged_sta)
            await run_task(task, self._state.manager)

            task = self.node.bridge._add_interface(bridged_sta,
                    vlan = vlan)
            await run_task(task, self._state.manager)

        task = self.node.bridge._add_interface(sta_tap, vlan = vlan)
        await run_task(task, self._state.manager)

    def _get_cmdline_params(self):

        # IP have not been assign, use AddressManager for simplicity since it
        # will remember the assignment
        # NOTE: here the IP address passed to emulator program is hardcoded with
        # a /24 mask(even if the associated IP with the station does not have a
        # /24 mask). This is not a problem at all because the netmask passed to
        # the emulator program has no impact on configuration in the emulator
        # program. Indeed, the IP routing table in the emulator program are
        # configured on a per address basis(one route per IP address) instead of
        # on a per prefix basis(one route per prefix). This guarantees the IP
        # routing will not change regardless of what netmask is. That is why we
        # can always safely pass a hardcoded /24 mask to the emulator program.

        sta_list = list() # list of identifiers
        sta_macs = list() # list of macs
        sta_taps = list()
        sta_ips = list()
        for station in self.stations:
            if not station.managed:
                interface = [i for i in station.interfaces if i.channel == self]
                assert len(interface) == 1
                interface = interface[0]

                sta_list.append(interface.name)
                sta_macs.append(interface.mac_address)
                sta_ips.append(interface.ip_address + '/24')
            else:
                identifier = self._sta_ifs[station]._state.uuid._uuid
                sta_list.append(identifier)

                mac = self._sta_ifs[station].mac_address
                sta_macs.append(mac)

                # Preallocate IP address
                ip = AddressManager().get_ip(self._sta_ifs[station]) + '/24'
                sta_ips.append(ip)

            tap = self._sta_taps[station].device_name
            sta_taps.append(tap)

        params = {
            # Name of the tap between NS3 and the base station
            'bs-tap'        : self._ap_tap.device_name,
            # Number of stations
            'n-sta'         : len(self._sta_taps),
            # List of the stations of the simulation
            'sta-list'      : ','.join(sta_list),
            # List of the taps between NS3 and the mobile stations
            'sta-taps'      : ','.join(sta_taps),
            # List of the macs of the mobile stations
            'sta-macs'      : ','.join(sta_macs),
            # X position of the Base Station
            'bs-x'          : 0, #self.ap.x,
            # Y position of the Base Station
            'bs-y'          : 0, #self.ap.y,
            # Experiment ID
            'experiment-id' : 'vicn',
            # Index of the base station
            'bs-name'       : self._ap_tap.device_name,
            # Base station IP address
            'bs-mac'        : self._ap_if.mac_address,
            # Control port for dynamically managing the stations movement
            'control-port'  : self.control_port,
            # Coma-separated list of stations' IP/netmask len
            'sta-ips'       : ','.join(sta_ips),
            # Base station IP/netmask len
            'bs-ip'         : AddressManager().get_ip(self._ap_if) + '/' +
                str(DEFAULT_NETMASK),
            'txBuffer'      : '800000',
            'isFading'      : 'true' if DEFAULT_FADING_ENABLED else 'false',
        }

        return ' '.join(['--{}={}'.format(k, v) for k, v in params.items()])

