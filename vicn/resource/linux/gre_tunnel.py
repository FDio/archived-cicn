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

import string

from netmodel.model.type            import Integer, String
from vicn.core.attribute            import Attribute
from vicn.core.resource             import Resource
from vicn.core.task                 import BashTask, inherit_parent, override_parent
from vicn.resource.channel          import Channel
from vicn.resource.linux.net_device import NetDevice, SlaveNetDevice

CMD_CREATE_GRE_TUNNEL='''
ip tunnel add {device_name} mode gre remote {dst} local {src} ttl 255
'''

class GREChannel(Channel):
    """
    Resource: GRETunnel
    """
    pass

class GREInterface(SlaveNetDevice):

    remote_address = Attribute(String, description ='',
                    mandatory = True)

    @override_parent
    def __create__(self):
        return BashTask(self.src_interface.node, CMD_CREATE_GRE_TUNNEL, {
            'device_name': self.device_name,
            'src': str(self.parent.ip4_address),
            'dst': self.remote_address})

class GRETunnel(Resource):

    src_interface = Attribute(NetDevice, description = 'source interface',
                mandatory = True)
    dst_interface = Attribute(NetDevice, description = 'destination interface',
                mandatory = True)

    @inherit_parent
    def __subresources__(self):
        channel = GREChannel()
        src = GREInterface(node=src_interface.node, device_name="gre0",
                                    parent=src_interface, channel=channel)
        dst = GREInterface(node=dst_interface.node, device_name="gre0",
                                    parent=dst_interface, channel=channel)
        return (src | dst) | channel
