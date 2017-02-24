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
from vicn.resource.linux.net_device import SlaveBaseNetDevice

CMD_CREATE_PARENT = 'ip link add name {netdevice.device_name} ' \
                    'link {netdevice.parent.device_name} '      \
                    'type {netdevice.netdevice_type} mode {netdevice.mode}'

class MacVtap(SlaveBaseNetDevice):
    """
    Resource: MacVtap

    Implements a MacVtap interface.
    """

    mode = Attribute(String, description = 'MACVTAP mode',
            default = 'bridge'),

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(self, *args, **kwargs)
        self.prefix = 'macvtap'
        self.netdevice_type = 'macvtap'

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __create__(self):
        return BashTask(self.node, CMD_CREATE_PARENT, {'netdevice': self})
