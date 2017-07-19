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

import logging

from vicn.core.task                import BashTask
from netmodel.model.object         import Object
from netmodel.model.attribute      import Attribute
from netmodel.model.query          import Query, ACTION_INSERT, ACTION_SELECT
from netmodel.model.type           import String
from netmodel.network.interface    import Interface, InterfaceState
from netmodel.network.packet       import Packet
from netmodel.network.prefix       import Prefix
from netmodel.util.misc            import lookahead

log = logging.getLogger(__name__)

class VICNBaseResource(Object):
    __type__ = 'vicn/'

    @classmethod
    def get(cls, query, interface):
        cb = interface._callback

        if query.object_name == 'script':
            predicates = query.filter.to_list()
            assert len(predicates) == 1
            _, _, name = predicates[0]
            script = '{}/{}'.format(interface._manager._base, name)

            task = BashTask(None, script)
            interface._manager.schedule(task)
            return

        elif query.object_name == 'gui':
            interface._manager._broadcast(query)
            return

        elif query.object_name == 'resource':
            resources = interface._manager.get_resources()
        else:
            _resources = interface._manager.by_type_str(query.object_name)
            resources = list()
            for resource in _resources:
                group_names = [r.name for r in resource.groups]
                resources.append(resource)

        if query.action == ACTION_SELECT:
            for resource, last in lookahead(resources):
                params = resource.get_attribute_dict(aggregates = True)
                params['id'] = resource._state.uuid._uuid
                params['type'] = resource.get_types()
                params['state'] = resource._state.state
                params['log'] = resource._state.log
                reply = Query(ACTION_INSERT, query.object_name, params = params)
                reply.last = last
                packet = Packet.from_query(reply, reply = True)
                cb(packet, ingress_interface = interface)
        else:
            log.warning("Unknown action in query {}".format(query))

        interface._manager._broadcast(query)

class L2Graph(Object):
    __type__ = 'vicn/l2graph'

    @classmethod
    def get(cls, query, interface):
        cb = interface._callback

        from vicn.resource.central import _get_l2_graph
        G = _get_l2_graph(interface._manager, with_managed=True)

        nodes = G.nodes()
        edges = G.edges()
        params = {'nodes': nodes, 'edges': edges}
        reply = Query(ACTION_INSERT, query.object_name, params = params)
        reply.last = True
        packet = Packet.from_query(reply, reply = True)
        cb(packet, ingress_interface = interface)

class VICNInterface(Interface):
    __interface__ = 'vicn'

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._manager = kwargs.pop('manager')

        # Resources
        resources = list()
        resources.extend(self._manager.get_resource_type_names())
        resources.append('resource')
        for resource in resources:
            class VICNResource(VICNBaseResource):
                __type__ = '{}'.format(resource.lower())
            self.register_object(VICNResource)

        self.register_object(L2Graph)
