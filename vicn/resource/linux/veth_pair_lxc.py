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
import random
import string

from vicn.core.attribute            import Attribute
from vicn.resource.linux.net_device import NetDevice, SlaveNetDevice
from vicn.core.task                 import BashTask, get_attributes_task
from vicn.core.task                 import override_parent
from vicn.core.attribute            import Attribute

# ip link add veth0 type veth peer name veth1

CMD_CREATE='''
# Create veth pair in the host node
ip link add name {interface.host.device_name} type veth peer name {tmp_name}
# The host interface will always be up...
ip link set dev {interface.host.device_name} up
# Move interface into container and rename it
ip link set dev {tmp_name} netns {pid} name {interface.device_name}
'''
CMD_UP='''
ip link set dev {interface.device_name} up
'''

# see:
# http://stackoverflow.com/questions/22780927/lxc-linux-containers-add-new-network-interface-without-restarting

class VethPairLxc(SlaveNetDevice):

    host = Attribute(NetDevice, description = 'Host interface',
            default = lambda x : x._default_host())

    def _default_host(self):
        if self.node.__class__.__name__ == 'LxcContainer':
            host = self.node.node
        else:
            host = self.node
        max_len = MAX_DEVICE_NAME_SIZE - len(self.node.name) - 1
        device_name = self.device_name[:max_len]

        return NetDevice(node = host,
                device_name = '{}-{}'.format(self.node.name, device_name),
                managed = False)

    def __create__(self):
        assert self.node.__class__.__name__ == 'LxcContainer'
        host = self.node.node
        pid = get_attributes_task(self.node, ['pid'])
        tmp_name = 'tmp-veth-' + ''.join(random.choice(string.ascii_uppercase \
                    + string.digits) for _ in range(5))
        create = BashTask(host, CMD_CREATE, {'tmp_name': tmp_name,
                'interface': self})
        up = BashTask(self.node, CMD_UP, {'interface': self})
        bridge = host.bridge_manager.add_interface(host.bridge.device_name,
                self.host.device_name)
        return ((pid @ create) > up) > bridge

    # ... IP and UP missing...
