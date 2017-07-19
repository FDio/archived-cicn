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

from vicn.core.address_mgr          import AddressManager
from vicn.core.attribute            import Attribute, Multiplicity
from vicn.core.exception            import ResourceNotFound
from vicn.core.requirement          import Requirement
from vicn.core.task                 import inline_task, inherit
from vicn.resource.channel          import Channel
from vicn.resource.interface        import Interface
from vicn.resource.linux.bridge_mgr import BridgeManager
from vicn.resource.linux.net_device import NetDevice

log = logging.getLogger(__name__)

class Bridge(Channel, NetDevice):
    """
    Resource: Bridge
    """
    node = Attribute(
            reverse_name = 'bridge',
            reverse_description = 'Main bridge',
            reverse_auto = 'true',
            multiplicity = Multiplicity.OneToOne,
            requirements = [
                Requirement('bridge_manager')
            ])
    device_name = Attribute(
            default = lambda self: self._state.manager.get('bridge_name'),
            mandatory = False)

    #--------------------------------------------------------------------------
    # Constructor / Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(self, *args, **kwargs)
        self.prefix = 'br'
        self.netdevice_type = 'bridge'

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inherit(Channel, Interface)
    def __create__(self):
        return self.node.bridge_manager.add_bridge(self.device_name)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def _add_interface(self, interface, vlan=None):
        """
        Returns:
            Task
        """
        return self.node.bridge_manager.add_interface(self.device_name,
                interface.device_name, vlan)

    def __method_add_interface__(self, interface, vlan=None):
        return self._add_interface(interface, vlan)

    def _remove_interface(self, interface):
        """
        Returns:
            Task
        """
        log.info('Removing interface {} from bridge {}'.format(
                interface.device_name, self.name))
        return self.node.bridge_manager.del_interface(self.device_name,
                interface.device_name)

