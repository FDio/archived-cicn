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

from netmodel.model.type                import String
from vicn.resource.icn.icn_application  import ICNApplication
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.resource.node                 import Node

class Producer(ICNApplication):
    """
    Resource: Producer
    """

    prefixes = Attribute(String, description = 'List of served prefixes',
            multiplicity = Multiplicity.OneToMany)
    #Overload to get producer list from node in CentralICN
    node = Attribute(Node,
            description = 'Node on which the producer is installed',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne,
            reverse_name = 'producers',
            key = True)
