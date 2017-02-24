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

from vicn.resource.ns3.emulated_channel import EmulatedChannel
from vicn.resource.linux.net_device     import NetDevice
from vicn.core.address_mgr              import AddressManager
from vicn.core.resource_mgr             import wait_resources
from vicn.core.task                     import async_task, run_task

class EmulatedWiFiChannel(EmulatedChannel):
    """EmulatedWiFiChannel Resource

    This resource uses the ns3 simulator to emulate an 80211n wireless channel.
    It assume no interference between base stations, and thus the emulated
    channel corresponds to a single base station.
    """
    __package_names__ = ['wifi-emulator']
    __app_name__ = 'wifi_emulator'

    #---------------------------------------------------------------------------
    # Attribute handlers
    #---------------------------------------------------------------------------

    async def _add_station(self, station):
        from vicn.resource.lxd.lxc_container    import LxcContainer
        from vicn.resource.linux.veth_pair      import VethPair
        from vicn.resource.linux.tap_device     import TapChannel
        from vicn.resource.linux.macvlan        import MacVlan

        interfaces = list()
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
        # sta_tap choosen because always there
        if sta_if:
            sta_if.set('channel', self)

            task = self.node.bridge._remove_interface(bridged_sta)
            await run_task(task, self._state.manager)
            task = self.node.bridge._add_interface(bridged_sta,
                    vlan = vlan)
            await run_task(task, self._state.manager)

        task = self.node.bridge._add_interface(sta_tap, vlan = vlan)
        await run_task(task, self._state.manager)


    def _get_cmdline_params(self, ):

        # sta-macs and sta-list for unmanaged stations
        sta_list = list() # list of identifiers
        sta_macs = list() # list of macs
        sta_taps = list()
        for station in self.stations:
            if not station.managed:
                interface = [i for i in station.interfaces if i.channel == self]
                assert len(interface) == 1
                interface = interface[0]

                sta_list.append(interface.name)
                sta_macs.append(interface.mac_address)
            else:
                identifier = self._sta_ifs[station]._state.uuid._uuid
                sta_list.append(identifier)

                mac = self._sta_ifs[station].mac_address
                sta_macs.append(mac)

            tap = self._sta_taps[station].device_name
            sta_taps.append(tap)

        params = {
            # Name of the tap between NS3 and the base station
            'bs-tap'        : self._ap_tap.device_name,
            # Number of stations
            'n-sta'         : len(self._sta_taps),
            # List of the stations of the simulation # identifiers
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
            # Base station MAC address
            'bs-mac'        : self._ap_if.mac_address,
            # Control port for dynamically managing the stations movement
            'control-port'  : self.control_port,
        }

        return ' '.join(['--{}={}'.format(k, v) for k, v in params.items()])
