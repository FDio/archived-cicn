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

from netmodel.model.type            import String, Integer
from netmodel.model.type            import Inet4Address, Inet6Address
from vicn.core.attribute            import Attribute
from vicn.core.resource             import BaseResource
from vicn.resource.interface        import Interface

class PhyInterface(Interface):
    """
    Resource: PhyInterface

    Physical network interface.
    """

    __type__ = BaseResource

    device_name = Attribute(String, description = 'Name of the DpdkDevice',
            mandatory = True)
    pci_address = Attribute(String, description = "Device's PCI bus address",
            mandatory = True)
    mac_address = Attribute(String, description = "Device's MAC address")
    ip4_address = Attribute(Inet4Address, description = "Device's IP address")
    ip6_address = Attribute(Inet6Address, description = "Device's IP address")
