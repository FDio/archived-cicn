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
from vicn.core.resource         import FactoryResource
from vicn.resource.application  import Application

class DnsServer(Application):
    """
    Resource: DnsServer
    """

    __type__ = FactoryResource

    node = Attribute(
            reverse_name = 'dns_server',
            reverse_auto = True,
            multiplicity = Multiplicity.OneToOne)
