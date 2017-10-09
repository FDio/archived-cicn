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
import networkx as nx

from netmodel.model.type                import String
from vicn.core.attribute                import Attribute, Reference
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import Resource
from vicn.core.task                     import inline_task
from vicn.resource.central              import _get_l2_graph, MAP_ROUTING_STRATEGY
from vicn.resource.icn.face             import Face
from vicn.resource.icn.route            import Route

log = logging.getLogger(__name__)


def _get_icn_graph(manager, groups):
    G = nx.Graph()
    for group in groups:
        # It's safer to iterate on node which we know are in the right groups,
        # while it might not be the case for the forwarders...
        for node in group.iter_by_type_str('node'):
            G.add_node(node._state.uuid)
            try:
                forwarder = node.forwarder
            except ResourceNotFound:
                continue
            for face in forwarder.faces:
                other_face = manager.by_uuid(face._internal_data['sibling_face'])
                other_node = other_face.node
                if G.has_edge(node._state.uuid, other_node._state.uuid):
                    continue
                map_node_face = { node._state.uuid: face._state.uuid,
                    other_node._state.uuid: other_face._state.uuid }
                G.add_edge(node._state.uuid, other_node._state.uuid,
                        map_node_face = map_node_face)

    return G

#-------------------------------------------------------------------------------

class ICNFaces(Resource):
    """
    Resource: ICNFaces

    Centralized ICN face creation.
    """
    protocol_name = Attribute(String)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __get__(self):
        raise ResourceNotFound # always create faces

    @inline_task
    def __create__(self):
        icn_faces = self._get_faces()
        for face in icn_faces:
            face.node.forwarder.faces << face

    def __delete__(self):
        raise NotImplementedError

    def __after__(self):
        return ("VPPInterface",)
    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _get_faces(self):
        """
        Face creation (heuristic: facemgr)

        Requires: at least direct IP links
        """
        faces = list()
        G = _get_l2_graph(self.groups)
        for src_node_uuid, dst_node_uuid, data in G.edges_iter(data = True):
            src_node = self._state.manager.by_uuid(src_node_uuid)
            dst_node = self._state.manager.by_uuid(dst_node_uuid)

            if not src_node.managed or not dst_node.managed:
                continue

            map_ = data['map_node_interface']
            src = self._state.manager.by_uuid(map_[src_node_uuid])
            if src.has_vpp_child:
                src = src.vppinterface
            dst = self._state.manager.by_uuid(map_[dst_node_uuid])
            if dst.has_vpp_child:
                dst = dst.vppinterface

            log.debug('{} -> {} ({} -> {})'.format(src_node_uuid,
                        dst_node_uuid, src.device_name, dst.device_name))

            face_cls = Face.from_protocol(self.protocol_name)
            if face_cls is None:
                raise NotImplementedError

            src_face = face_cls(protocol    = self.protocol_name,
                                owner       = self,
                                node        = src_node,
                                src         = src,
                                dst         = dst)
            dst_face = face_cls(protocol    = self.protocol_name,
                                owner       = self,
                                node        = dst_node,
                                src         = dst,
                                dst         = src)


            # We key the sibling face for easier building of the ICN graph
            src_face._internal_data['sibling_face'] = dst_face._state.uuid
            dst_face._internal_data['sibling_face'] = src_face._state.uuid

            faces.append(src_face)
            faces.append(dst_face)

        return faces

#------------------------------------------------------------------------------

class ICNRoutes(Resource):
    """
    Resource: Routes

    Centralized ICN route computation.
    """

    routing_strategy = Attribute(String)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __get__(self):
        raise ResourceNotFound # always create routes

    @inline_task
    def __create__(self):
        icn_routes = self._get_icn_routes()
        for route in icn_routes:
            route.node.forwarder.routes << route

    def __delete__(self):
        raise NotImplementedError

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _get_prefix_origins(self):
        origins = dict()
        for group in self.groups:
            for node in group.iter_by_type_str('node'):
                node_uuid = node._state.uuid
                if not node_uuid in origins:
                    origins[node_uuid] = list()
                for producer in node.producers:
                    origins[node_uuid].extend(producer.prefixes)
        return origins

    def _get_icn_routes(self):
        strategy = MAP_ROUTING_STRATEGY.get(self.routing_strategy)

        G = _get_icn_graph(self._state.manager, self.groups)
        origins = self._get_prefix_origins()

        routes = list()
        for src, prefix, dst in strategy(G, origins):
            src_node = self._state.manager.by_uuid(src)
            if not src_node.managed:
                continue
            data = G.get_edge_data(src, dst)

            map_ = data['map_node_face']
            next_hop_face = map_[src]

            route = Route(node   = src,
                             owner  = self,
                             prefix = prefix,
                             face   = next_hop_face)
            routes.append(route)

        return routes

#------------------------------------------------------------------------------

class CentralICN(Resource):
    """
    Resource: CentralICN

    Central ICN management (main resource)
    """

    # Choices: spt, max_flow
    icn_routing_strategy = Attribute(String,
            description = 'ICN routing strategy',
            default = 'spt')
    face_protocol = Attribute(String,
            description = 'Protocol used to create faces',
            default = 'ether')

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        """
        We need to wait for IP configuration in order to be able to build
        overload ICN faces, and producers for prefix origins.
        """
        return ('CentralIP',)

    def __subresources__(self):
        icn_faces = ICNFaces(owner = self, protocol_name = self.face_protocol,
                groups = Reference(self, 'groups'))
        icn_routes = ICNRoutes(owner = self,
                routing_strategy = self.icn_routing_strategy,
                groups = Reference(self, 'groups'))
        return icn_faces > icn_routes
