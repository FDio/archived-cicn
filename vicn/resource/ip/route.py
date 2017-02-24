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

from netmodel.model.type        import String
from vicn.resource.node         import Node
from vicn.core.attribute        import Attribute
from vicn.core.resource         import Resource
from vicn.resource.interface    import Interface

class IPRoute(Resource):
    node = Attribute(Node, mandatory = True)
    ip_address = Attribute(String, mandatory = True)
    interface = Attribute(Interface, mandatory = True)
    gateway = Attribute(String)

    # FIXME Temp hack for VPP, migrate this to an ARP table resource
    mac_address = Attribute(String)
