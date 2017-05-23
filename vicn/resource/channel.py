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

from netmodel.model.type    import String
from vicn.core.resource     import Resource
from vicn.core.attribute    import Attribute
from vicn.core.task         import EmptyTask
from vicn.resource.ip_assignment    import Ipv6Assignment, Ipv4Assignment

class Channel(Resource):
    """
    Resource: Channel
    """

    #--------------------------------------------------------------------------
    # Public API
    #--------------------------------------------------------------------------

    def __after_init__(self):
        return ("IpAssignment",)

    def get_remote_name(self, name):
        if len(self._interfaces) != 2:
            return None
        return next(x for x in self._interfaces if x.get_name() != name)

    def get_sortable_name(self):
        """
        This method is used to sort channel during IP assignment. This is
        necessary to get the same IP configuration on the same experiment.
        """
        ret = "{:03}".format(len(self.interfaces))
        ret = ret + ''.join(sorted(map(lambda x : x.node.name, self.interfaces)))
        return ret
