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

from vicn.core.attribute        import Attribute, Multiplicity
from netmodel.model.type        import Bool
from vicn.core.resource         import Resource
from vicn.resource.node         import Node
from vicn.resource.channel      import Channel

class Interface(Resource):
    """
    Resource: Interface
    """

    node = Attribute(Node, description = 'Node to which the interface belongs',
            multiplicity = Multiplicity.ManyToOne,
            reverse_name = 'interfaces',
            mandatory = True)
    channel = Attribute(Channel, description = 'Channel to which the interface is attached',
            multiplicity = Multiplicity.ManyToOne,
            reverse_name = 'interfaces')
    promiscuous = Attribute(Bool, description = 'Promiscuous mode',
                default = False)
    up = Attribute(Bool, description = 'Interface up/down status',
                default = True)
    monitored = Attribute(Bool, default = True)

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # Flag to check whether vpp uses that interface
        self.has_vpp_child = False
