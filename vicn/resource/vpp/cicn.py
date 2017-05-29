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

from netmodel.model.type            import Integer, Bool
from vicn.core.attribute            import Attribute
from vicn.core.exception            import ResourceNotFound
from vicn.core.requirement          import Requirement
from vicn.core.resource_mgr         import wait_resource_task
from vicn.core.task                 import async_task, task, BashTask, EmptyTask
from vicn.resource.icn.forwarder    import Forwarder
from vicn.resource.node             import Node
from vicn.resource.vpp.vpp_commands import CMD_VPP_ENABLE_PLUGIN
from vicn.resource.vpp.vpp_commands import CMD_VPP_CICN_GET
from vicn.resource.vpp.vpp_commands import CMD_VPP_ADD_ICN_FACE
from vicn.resource.vpp.vpp_commands import CMD_VPP_ADD_ICN_ROUTE
from vicn.resource.vpp.vpp_commands import CMD_VPP_CICN_GET_CACHE_SIZE
from vicn.resource.vpp.vpp_commands import CMD_VPP_CICN_SET_CACHE_SIZE

_ADD_FACE_RETURN_FORMAT = "Face ID: [0-9]+"

def check_face_id_return_format(data):
    prog = re.compile(_ADD_FACE_RETURN_FORMAT)
    return prog.match(data)

def parse_face_id(data):
    return data.partition(':')[2]

class CICNForwarder(Forwarder):
    """
    NOTE: Based on the Vagrantfile, we recommend a node with mem=4096, cpu=4
    """

    node = Attribute(Node,
            mandatory=True,
            requirements = [Requirement('vpp')],
            reverse_name='cicn')
    numa_node = Attribute(Integer,
            description = 'Numa node on which vpp will run',
            default = None)
    core = Attribute(Integer,
            description = 'Core belonging the numa node on which vpp will run',
            default = None)
    enable_worker = Attribute(Bool,
            description = 'Enable one worker for packet processing',
            default = False)

    #__packages__ = ['vpp-plugin-cicn']

    def __after__(self):
        return ['CentralICN']

    def __get__(self):
        def parse(rv):
            if rv.return_value > 0 or 'cicn: not enabled' in rv.stdout:
                raise ResourceNotFound
        return BashTask(self.node, CMD_VPP_CICN_GET,
                lock = self.node.vpp.vppctl_lock, parse=parse)

    def __create__(self):

        #self.node.vpp.plugins.append("cicn")
        lock = self.node.vpp.vppctl_lock
        create_task = BashTask(self.node, CMD_VPP_ENABLE_PLUGIN,
                {'plugin' : 'cicn'}, lock = lock)

        face_task = EmptyTask()
        route_task = EmptyTask()

        def parse_face(rv, face):
            if check_face_id_return_format(rv.stdout):
                face.id = parse_face_id(rv.stdout)
            return {}

        for face in self.faces:
            face_task = face_task > BashTask(self.node, CMD_VPP_ADD_ICN_FACE,
                    {'face':face},
                    parse = (lambda x : parse_face(x, face)), lock = lock)

        if not self.routes:
            from vicn.resource.icn.route import Route
            for route in self._state.manager.by_type(Route):
                if route.node is self.node:
                    self.routes.append(route)
        for route in self.routes:
            route_task = route_task > BashTask(self.node,
                    CMD_VPP_ADD_ICN_ROUTE, {'route' : route}, lock = lock)

        return (wait_resource_task(self.node.vpp) > create_task) > (face_task > route_task)

    # Nothing to do
    __delete__ = None

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    # Force local update

    _add_faces = None
    _remove_faces = None
    _get_faces = None
    _set_faces = None

    _add_routes = None
    _remove_routes = None
    _get_routes = None
    _set_routes = None

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _set_cache_size(self):
        return BashTask(self.node, CMD_VPP_CICN_SET_CACHE_SIZE, {'self': self},
                lock = self.node.vpp.vppctl_lock)

    def _get_cache_size(self):
        def parse(rv):
            return int(rv.stdout)
        return BashTask(self.node, CMD_VPP_CICN_GET_CACHE_SIZE, parse=parse,
                lock = self.node.vpp.vppctl_lock)
