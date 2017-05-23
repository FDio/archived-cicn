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

from vicn.core.resource        import Resource
from vicn.core.attribute       import Attribute, Multiplicity

class Group(Resource):
    resources = Attribute(Resource, description = 'Resources belonging to the group',
	    multiplicity = Multiplicity.ManyToMany,
            default = [],
            reverse_name = 'groups',
            reverse_description = 'Groups to which the resource belongs')

    def iter_by_type(self, type):
        for r in self.resources:
            if isinstance(r, type):
                yield r

    def iter_by_type_str(self, typestr):
        cls = self._state.manager._available.get(typestr.lower())
        if not cls:
            return list()
        return self.iter_by_type(cls)
