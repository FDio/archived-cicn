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

from vicn.core.attribute                import Attribute
from vicn.core.task                     import inline_task
from vicn.resource.channel              import Channel
from vicn.resource.linux.phy_interface  import PhyInterface

class PhyLink(Channel):
    """
    Resource: PhyLink

    Physical Link to inform the orchestrator about Layer2 connectivity.
    """

    src = Attribute(PhyInterface, description = 'Source interface', 
            mandatory = True)
    dst = Attribute(PhyInterface, description = 'Destination interface', 
            mandatory = True)

    @inline_task
    def __initialize__(self):
        self.src.set('channel', self)
        self.dst.set('channel', self)
