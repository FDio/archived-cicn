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

from netmodel.model.type                import Integer, String
from vicn.core.attribute                import Attribute
from vicn.resource.linux.phy_interface  import PhyInterface

class DpdkDevice(PhyInterface):
    """
    Resource: DpdkDevice

    A DpdkDevice is a physical net device supported by Dpdk and with parameters
    specific to VPP.
    """
    numa_node = Attribute(Integer, 
            description = 'NUMA node on the same PCI bus as the DPDK card')
    socket_mem = Attribute(Integer, 
            description = 'Memory used by the vpp forwarder', 
            default = 512)
    mac_address = Attribute(String)
