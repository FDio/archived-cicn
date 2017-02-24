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

import re
from itertools              import cycle

from netmodel.model.type    import BaseType
from vicn.core.resource     import Resource
from vicn.core.attribute    import Attribute, Multiplicity
from vicn.core.task         import BashTask
from vicn.resource.node     import Node

PATTERN_LSCPU_NUMA = 'NUMA node[0-9]+ CPU\(s\)'
CMD_LSCPU = 'lscpu'

class CycleType(BaseType, cycle):
    """
    Type: CycleType
    """
    pass

#------------------------------------------------------------------------------ 

def parse_lscpu_line(line):
    #Format: NUMA node0 CPU(s):     0-17,36-53
    line = line.split(':')[1]
    #line = 0-17,36,53

    def limits_to_list(string):
        limits = string.split('-')
        lower_limit = int(limits[0])
        #Removes core 0 as it is used the most often by the kernl
        if lower_limit is 0 : lower_limit = 1
        return cycle(range(lower_limit, int(limits[1])))
    return cycle(map(limits_to_list, line.split(',')))

def parse_lscpu_rv(rv):
    ret = []
    for line in rv.stdout.splitlines():
        if re.search(PATTERN_LSCPU_NUMA, line):
            ret.append(parse_lscpu_line(line))
    return ret

#------------------------------------------------------------------------------ 

class NumaManager(Resource):
    """
    Resource: NumaManager
    """

    node = Attribute(Node,
            mandatory = True,
            multiplicity = Multiplicity.OneToOne,
            reverse_auto = True,
            reverse_name = 'numa_mgr')
    numa_repartitor = Attribute(CycleType, 
            description = 'Tool to separate cores/CPUs/sockets',
            multiplicity = Multiplicity.OneToMany, 
            ro = True)

    #-------------------------------------------------------------------------- 
    # Resource lifecycle
    #-------------------------------------------------------------------------- 

    __create__ = None
    __delete__ = None

    #-------------------------------------------------------------------------- 
    # Constructor and Accessors
    #-------------------------------------------------------------------------- 

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.current_numa_node = 0

    #-------------------------------------------------------------------------- 
    # Attributes
    #-------------------------------------------------------------------------- 

    def _get_numa_repartitor(self):
        return BashTask(self.node, CMD_LSCPU, parse=parse_lscpu_rv)

    #-------------------------------------------------------------------------- 
    # Public API 
    #-------------------------------------------------------------------------- 

    def get_numa_core(self, numa_node=None):
        if numa_node is None:
            numa_node = self.current_numa_node
            self.current_numa_node = (self.current_numa_node+1) % \
                    len(self.numa_repartitor)
        numa_list = self.numa_repartitor[numa_node]

        socket = next(numa_list)
        return numa_node, next(socket)

    def get_number_of_numa(self):
        return len(self.numa_repartitor)
