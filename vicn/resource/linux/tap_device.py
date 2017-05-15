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

from netmodel.model.type            import String
from vicn.core.attribute            import Attribute
from vicn.core.task                 import BashTask
from vicn.resource.linux.net_device import BaseNetDevice, IPV4, IPV6, CMD_FLUSH_IP

CMD_CREATE='ip tuntap add name {netdevice.device_name} mode tap'
#CMD_SET_IP_ADDRESS='ip -{version} addr add dev {netdevice.device_name} 0.0.0.0'

class TapDevice(BaseNetDevice):

    def __init__(self, *args, **kwargs):
        super().__init__(self, *args, **kwargs)
        self.prefix = 'tap'
        self.netdevice_type = 'tap'

    def __create__(self):
        return BashTask(self.node, CMD_CREATE, {'netdevice': self})

##mengueha: do we actually need that?
#    def _set_ip4_address(self):
#        if self.ip4_address is None:
#            # Unset IP
#            return BashTask(self.node, CMD_FLUSH_IP, 
#                    {'device_name': self.device_name})
#        return BashTask(self.node, CMD_SET_IP_ADDRESS, 
#                {'netdevice': self})
#
#    def _set_ip6_address(self):
#        if self.ip6_address is None:
#            # Unset IP
#            return BashTask(self.node, CMD_FLUSH_IP, 
#                    {'ip_version': IPV6, 'device_name': self.device_name})
#        return BashTask(self.node, CMD_SET_IP_ADDRESS, 
#                {'netdevice': self})

class TapChannel(TapDevice):
    station_name = Attribute(String)
    channel_name = Attribute(String)
