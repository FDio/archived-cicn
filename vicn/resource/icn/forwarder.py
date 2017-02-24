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

from abc                    import ABC
from enum                   import Enum

from netmodel.model.type                import Integer, String
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.resource                 import FactoryResource
from vicn.resource.icn.icn_application  import ICNApplication
from vicn.resource.icn.face             import Face
from vicn.resource.icn.route            import Route

DEFAULT_CACHE_SIZE = 1000 # pk
DEFAULT_CACHE_POLICY = 'LRU'
DEFAULT_STRATEGY = 'best-route'

class Forwarder(ICNApplication, ABC):
    """
    Resource: Forwarder
    """

    __type__ = FactoryResource

    faces = Attribute(Face, description = 'ICN ffaces of the forwarder',
            multiplicity = Multiplicity.OneToMany,
            reverse_name = 'forwarder')
    routes = Attribute(Route, description = 'Routes in the ICN FIB', 
            multiplicity = Multiplicity.OneToMany,
            reverse_name = 'forwarder')
    cache_size = Attribute(Integer, 
            description = 'Size of the cache (in chunks)',
            default = DEFAULT_CACHE_SIZE)
    cache_policy = Attribute(String, description = 'Cache policy', 
            default = DEFAULT_CACHE_POLICY)
    strategy = Attribute(String, description = 'Forwarding Strategy', 
            default = DEFAULT_STRATEGY)
    config_file = Attribute(String, description = 'Configuration file')
    port = Attribute(Integer, description = 'Default listening port', 
            default = lambda self: self._get_default_port())
    log_file = Attribute(String, description = 'Log file')

    # Overloaded attributes

    node = Attribute(
        reverse_name = 'forwarder',
        reverse_description = 'ICN forwarder attached to the node',
        reverse_auto = True,
        multiplicity = Multiplicity.OneToOne)
