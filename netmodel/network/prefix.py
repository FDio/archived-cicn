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

class Prefix:
    def __init__(self, object_name = None, filter = None, field_names = None, 
            aggregate = None):
        self.object_name = object_name
        self.filter = filter
        self.field_names = field_names
        self.aggregate = aggregate

    def __hash__(self):
        return hash(self.get_tuple())

    def get_tuple(self):
        return (self.object_name, self.filter, self.field_names, 
                self.aggregate)

    def __repr__(self):
        return '<Prefix {}>'.format(self.get_tuple())

    __str__ = __repr__
