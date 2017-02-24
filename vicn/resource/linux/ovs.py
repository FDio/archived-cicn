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

from vicn.core.task                     import BashTask
from vicn.resource.linux.bridge_mgr     import BridgeManager

CMD_ADD_BRIDGE = '''
ovs-vsctl --may-exist add-br {bridge_name}
ip link set dev {bridge_name} up
'''

CMD_DEL_BRIDGE = 'ovs-vsctl --if-exists del-br {bridge_name}'

CMD_ADD_INTERFACE = 'ovs-vsctl --may-exist add-port {bridge_name} ' \
                    '{interface_name}'
CMD_ADD_INTERFACE_VLAN = CMD_ADD_INTERFACE + ' tag={vlan}'
CMD_DEL_INTERFACE = 'ovs-vsctl --if-exists del-port {bridge_name} ' \
                    '{interface_name}'

class OVS(BridgeManager):
    """
    Resource: OVS

    OpenVSwitch bridge manager
    """

    __package_names__ = ['openvswitch-switch']

    #---------------------------------------------------------------------------
    # BridgeManager API
    #---------------------------------------------------------------------------

    def add_bridge(self, bridge_name):
        return BashTask(self.node, CMD_ADD_BRIDGE, 
                {'bridge_name': bridge_name},
                output = False, as_root = True)

    def del_bridge(self, bridge_name):
        return BashTask(self.node, CMD_DEL_BRIDGE, 
                {'bridge_name': bridge_name},
                output = False, as_root = True)

    def add_interface(self, bridge_name, interface_name, vlan=None):
        cmd = CMD_ADD_INTERFACE_VLAN if vlan is not None else CMD_ADD_INTERFACE
        return BashTask(self.node, cmd, {'bridge_name': bridge_name,
                'interface_name': interface_name, 'vlan': vlan},
                output = False, as_root = True)

    def del_interface(self, bridge_name, interface_name, vlan=None):
        return BashTask(self.node, CMD_DEL_INTERFACE, 
                {'bridge_name': bridge_name, 'interface_name': interface_name,
                 'vlan': vlan},
                output = False, as_root = True)
