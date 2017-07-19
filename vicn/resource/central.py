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
import os

from netmodel.util.misc                 import pairwise
from vicn.core.attribute                import Attribute, Reference
from vicn.resource.channel              import Channel

log = logging.getLogger(__name__)

#------------------------------------------------------------------------------
# Routing strategies
#------------------------------------------------------------------------------

def routing_strategy_spt(G, origins, weight_key = None):
    """Routing strategy : Shortest path tree

    This routing strategy uses the Dijkstra algorithm on an undirected graph
    to build the shortest path tree towards all origin prefixes.

    NOTE: weights are currently unsupported by this strategy.

    Args:
        G (nx.Graph): network graph
        origins (dict): dictionary mapping nodes to the set of prefixes they
                        are origins for
        weight_key (str): key corresponding to weight key in edge data. None
                          assumes all weights have unit cost

    Returns:
        generator : returning triplets (source, prefix, next hop)
    """
    assert weight_key is None

    origin_nodes = origins.keys()
    seen = set()
    for dst_node in origin_nodes:
        if not G.has_node(dst_node):
            continue
        sssp = nx.shortest_path(G, target = dst_node)
        # Notes from the documentation:
        #  - If only the target is specified, return a dictionary keyed by
        # sources with a list of nodes in a shortest path from one of the
        # sources to the target.
        #  - All returned paths include both the source and target in the
        # path.
        for _, path in sssp.items():
            if len(path) == 1:
                # Local prefix
                continue
            for s, d in pairwise(path):
                for prefix in origins[dst_node]:
                    t = (s, prefix, d)
                    if t in seen:
                        continue
                    seen.add(t)
                    yield t

def routing_strategy_max_flow(G, origins, weight_key = 'capacity'):
    """Routing strategy : Maximum Flow

    TODO

    Args:
        G (nx.Graph): network graph
        origins (dict): dictionary mapping nodes to the set of prefixes they
                        are origins for
        weight_key (str): key corresponding to weight key in edge data. None
                          assumes all weights have unit cost

    Returns:
        generator : returning triplets (source, prefix, next hop)
    """
    assert weight_key is None

    origin_nodes = origins.keys()
    for dst_node in origin_nodes:
        if not G.has_node(dst_node):
            continue
        for src_node in G.nodes:
            if src_node == dst_node:
                continue
            if not G.has_node(src_node):
                continue
            _, flow_dict = nx.maximum_flow(G, src_node, dst_node,
                    capacity=weight_key)

            # Notes from the documentation:
            # https://networkx.github.io/documentation/networkx-1.10/reference/
            #       generated/networkx.algorithms.flow.maximum_flow.html
            #  - flow_dict (dict) – A dictionary containing the value of the
            # flow that went through each edge.
            for s, d_map in flow_dict.items():
                for d, flow in d_map.items():
                    if flow == 0:
                        continue
                    for prefix in origins[dst_node]:
                        yield s, prefix, d

MAP_ROUTING_STRATEGY = {
    'spt'       : routing_strategy_spt,
    'max_flow'  : routing_strategy_max_flow,
}

#------------------------------------------------------------------------------
# L2 and L4/ICN graphs
#------------------------------------------------------------------------------

def _get_l2_graph(groups):
    """
    We iterate on all the channels that belong to the same groups as the
    resources.

    NOTE: We have to make sure the nodes also belong to the group.
    """
    G = nx.Graph()

    for group in groups:
        for channel in group.iter_by_type_str('channel'):
            if channel.has_type('emulatedchannel'):
                src = channel._ap_if
                # XXX bug in reverse collections, resources and not UUIDs seem to be stored inside
                if group.name not in [x.name for x in src.node.groups]:
                    continue
                for dst in channel._sta_ifs.values():
                    if group.name not in [x.name for x in dst.node.groups]:
                        continue
                    if G.has_edge(src.node._state.uuid, dst.node._state.uuid):
                        continue

                    map_node_interface = { src.node._state.uuid : src._state.uuid,
                        dst.node._state.uuid: dst._state.uuid}
                    G.add_edge(src.node._state.uuid, dst.node._state.uuid,
                            map_node_interface = map_node_interface)
            else:
                # This is for a normal Channel
                for src_it in range(0, len(channel.interfaces)):
                    src = channel.interfaces[src_it]
                    if group.name not in [x.name for x in src.node.groups]:
                        continue

                    # Iterate over the remaining interface to create all the
                    # possible combination
                    for dst_it in range(src_it+1,len(channel.interfaces)):
                        dst = channel.interfaces[dst_it]
                        if group.name not in [x.name for x in dst.node.groups]:
                            continue

                        if G.has_edge(src.node._state.uuid, dst.node._state.uuid):
                            continue
                        map_node_interface = {
                            src.node._state.uuid : src._state.uuid,
                            dst.node._state.uuid: dst._state.uuid}
                        G.add_edge(src.node._state.uuid, dst.node._state.uuid,
                                map_node_interface = map_node_interface)
    return G
