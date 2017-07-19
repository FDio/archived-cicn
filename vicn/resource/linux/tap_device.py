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
from vicn.core.task                 import BashTask, override_parent
from vicn.resource.linux.net_device import NetDevice, IPV4, IPV6, CMD_FLUSH_IP

CMD_CREATE='ip tuntap add name {netdevice.device_name} mode tap'

class TapDevice(NetDevice):

    def __init__(self, *args, **kwargs):
        super().__init__(self, *args, **kwargs)
        self.prefix = 'tap'
        self.netdevice_type = 'tap'

    @override_parent
    def __create__(self):
        return BashTask(self.node, CMD_CREATE, {'netdevice': self})

class TapChannel(TapDevice):
    station_name = Attribute(String)
    channel_name = Attribute(String)
