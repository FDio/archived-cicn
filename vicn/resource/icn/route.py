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

from netmodel.model.type    import Integer, String
from vicn.core.attribute    import Attribute
from vicn.core.resource     import Resource
from vicn.resource.icn.face import Face
from vicn.resource.node     import Node

class Route(Resource):
    node = Attribute(Node, mandatory = True)
    prefix = Attribute(String, mandatory = True)
    face = Attribute(Face, description = "face used to forward interests", 
            mandatory=True)
    cost = Attribute(Integer, default=1)

    def __repr__(self):
        return '<Route {} {} on node {}>'.format(self.prefix, self.face, 
                self.node.name)
    
    __str__ = __repr__
