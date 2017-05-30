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

from netmodel.model.type                import String, Integer
from netmodel.util.misc                 import pairwise
from vicn.core.attribute                import Attribute, Reference
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import Resource
from vicn.core.task                     import async_task, inline_task
from vicn.core.task                     import EmptyTask, BashTask
from vicn.resource.channel              import Channel
from vicn.resource.ip.route             import IPRoute
from vicn.resource.group                import Group
from vicn.resource.icn.forwarder        import Forwarder
from vicn.resource.icn.face             import L2Face, L4Face, FaceProtocol
from vicn.resource.icn.producer         import Producer
from vicn.resource.icn.route            import Route as ICNRoute
from vicn.resource.lxd.lxc_container    import LxcContainer
from vicn.resource.node                 import Node
from vicn.resource.ip_assignment        import Ipv4Assignment, Ipv6Assignment

log = logging.getLogger(__name__)

TMP_DEFAULT_PORT = 6363

CMD_CONTAINER_SET_DNS = 'echo "nameserver {ip_dns}" | ' \
                        'resolvconf -a {interface_name}'

# For host
CMD_NAT = '\n'.join([
    'iptables -t nat -A POSTROUTING -o {interface_name} -s {network} ' \
    '! -d {network} -j MASQUERADE',
    'echo 1 > /proc/sys/net/ipv4/ip_forward'
])

# For containers
CMD_IP_FORWARD = 'echo 1 > /proc/sys/net/ipv4/ip_forward'

HOST_FILE = "hosts.vicn"

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
        for src_node in G.nodes:
            if src_node == dst_node:
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

def _get_l2_graph(groups, with_managed = False):
    G = nx.Graph()
#    for node in manager.by_type(Node):
#        G.add_node(node._state.uuid)

    for group in groups:
        for channel in group.iter_by_type_str('channel'):
            if channel.has_type('emulatedchannel'):
                src = channel._ap_if
                for dst in channel._sta_ifs.values():
                    if not with_managed and (not src.managed or not dst.managed):
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

                    # Iterate over the remaining interface to create all the
                    # possible combination
                    for dst_it in range(src_it+1,len(channel.interfaces)):
                        dst = channel.interfaces[dst_it]

                        if not with_managed and (not src.managed or
                                not dst.managed):
                            continue
                        if G.has_edge(src.node._state.uuid, dst.node._state.uuid):
                            continue
                        map_node_interface = {
                            src.node._state.uuid : src._state.uuid,
                            dst.node._state.uuid: dst._state.uuid}
                        G.add_edge(src.node._state.uuid, dst.node._state.uuid,
                                map_node_interface = map_node_interface)
    return G

def _get_icn_graph(manager, groups):
    G = nx.Graph()
    for group in groups:
        # It's safer to iterate on node which we know are in the right groups,
        # while it might not be the case for the forwarders...
        for node in group.iter_by_type_str('node'):
            G.add_node(node._state.uuid)
            for face in node.forwarder.faces:
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

class IPRoutes(Resource):
    """
    Resource: IPRoutes

    Centralized IP route computation.
    """
    routing_strategy = Attribute(String)

    def __after__(self):
        return ("IpAssignment",)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    @inline_task
    def __create__(self):
        routes = list()
        pre_routes, routes = self._get_ip_routes()
        routes.extend(pre_routes)
        routes.extend(routes)
        for route in routes:
            route.node.routing_table.routes << route

    def __delete__(self):
        raise NotImplementedError

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _get_ip_origins(self):
        origins = dict()
        for group in self.groups:
            for channel in group.iter_by_type_str('channel'):
                for interface in channel.interfaces:
                    node_uuid = interface.node._state.uuid
                    if not node_uuid in origins:
                        origins[node_uuid] = list()
                    ip4 = interface.ip4_address
                    origins[node_uuid].append(interface.ip4_address)
                    if interface.ip6_address:
                        ip6 = interface.ip6_address
                        origins[node_uuid].append(interface.ip6_address)
        return origins

    def _get_ip_routes(self):
        if self.routing_strategy == 'pair':
            return [], self._get_pair_ip_routes()

        strategy = MAP_ROUTING_STRATEGY.get(self.routing_strategy)

        G = _get_l2_graph(self.groups)
        origins = self._get_ip_origins()

        # node -> list(origins for which we have routes)
        ip_routes = dict()

        pre_routes = list()
        routes = list()
        for src, prefix, dst in strategy(G, origins):
            data = G.get_edge_data(src, dst)

            map_ = data['map_node_interface']
            next_hop_interface = map_[src]


            next_hop_ingress    = self._state.manager.by_uuid(map_[dst])
            src_node = self._state.manager.by_uuid(src)

            mac_addr = None
            if ((hasattr(next_hop_ingress, 'vpp') and
                        next_hop_ingress.vpp is not None) or
                    (hasattr(src_node, 'vpp') and src_node.vpp is not None)):
                mac_addr = next_hop_ingress.mac_address

            # Avoid duplicate routes due to multiple paths in the network
            if not src_node in ip_routes:
                ip_routes[src_node] = list()
            if prefix in ip_routes[src_node]:
                continue

            #FIXME: should test for IP format
            ip_version = 6 if ":" in prefix else 4
            next_hop_ingress_ip = (next_hop_ingress.ip6_address if ip_version is 6 else
                    next_hop_ingress.ip4_address)
            if prefix == next_hop_ingress_ip:
                # Direct route on src_node.name :
                # route add [prefix] dev [next_hop_interface_.device_name]
                route = IPRoute(node     = src_node,
                                managed    = False,
                                owner      = self,
                                ip_address = prefix,
                                mac_address = mac_addr,
                                ip_version = ip_version,
                                interface  = next_hop_interface)
            else:
                # We need to be sure we have a route to the gw from the node
                if not next_hop_ingress_ip in ip_routes[src_node]:
                    pre_route = IPRoute(node    = src_node,
                                    managed     = False,
                                    owner       = self,
                                    ip_address  = next_hop_ingress_ip,
                                    ip_version  = ip_version,
                                    mac_address = mac_addr,
                                    interface   = next_hop_interface)
                    ip_routes[src_node].append(next_hop_ingress_ip)
                    pre_routes.append(pre_route)

                # Route on src_node.name:
                # route add [prefix] dev [next_hop_interface_.device_name]
                #    via [next_hop_ingress.ip_address]
                route = IPRoute(node        = src_node,
                                managed     = False,
                                owner       = self,
                                ip_address  = prefix,
                                ip_version  = ip_version,
                                interface   = next_hop_interface,
                                mac_address = mac_addr,
                                gateway     = next_hop_ingress_ip)
            ip_routes[src_node].append(prefix)
            routes.append(route)
        return pre_routes, routes

    def _get_pair_ip_routes(self):
        """
        IP routing strategy : direct routes only
        """
        routes = list()
        G = _get_l2_graph(self.groups)
        for src_node_uuid, dst_node_uuid, data in G.edges_iter(data = True):
            src_node = self._state.manager.by_uuid(src_node_uuid)
            dst_node = self._state.manager.by_uuid(dst_node_uuid)

            map_ = data['map_node_interface']
            src = self._state.manager.by_uuid(map_[src_node_uuid])
            dst = self._state.manager.by_uuid(map_[dst_node_uuid])

            log.debug('[IP ROUTE] NODES {}/{}/{} -> {}/{}/{}'.format(
                        src_node.name, src.device_name, src.ip4_address,
                        dst_node.name, dst.device_name, dst.ip4_address))
            log.debug('[IP ROUTE] NODES {}/{}/{} -> {}/{}/{}'.format(
                        dst_node.name, dst.device_name, dst.ip4_address,
                        src_node.name, src.device_name, src.ip4_address))

            route = IPRoute(node        = src_node,
                            managed     = False,
                            owner       = self,
                            ip_address  = dst.ip4_address,
                            mac_address = dst.mac_address,
                            interface   = src)
            routes.append(route)
            route = IPRoute(node        = src_node,
                            managed     = False,
                            owner       = self,
                            ip_address  = dst.ip6_address,
                            ip_version  = 6,
                            mac_address = dst.mac_address,
                            interface   = src)
            routes.append(route)

            route = IPRoute(node       = dst_node,
                            managed    = False,
                            owner      = self,
                            ip_address = src.ip4_address,
                            mac_address = src.mac_address,
                            interface  = dst)
            routes.append(route)
            route = IPRoute(node       = dst_node,
                            managed    = False,
                            owner      = self,
                            ip_address = src.ip6_address,
                            ip_version = 6,
                            mac_address = src.mac_address,
                            interface  = dst)
            routes.append(route)

        return routes

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

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _get_faces(self):
        """
        Face creation (heuristic: facemgr)

        Requires: at least direct IP links
        """
        protocol = FaceProtocol.from_string(self.protocol_name)

        faces = list()
        G = _get_l2_graph(self.groups)
        for src_node_uuid, dst_node_uuid, data in G.edges_iter(data = True):
            src_node = self._state.manager.by_uuid(src_node_uuid)
            dst_node = self._state.manager.by_uuid(dst_node_uuid)

            map_ = data['map_node_interface']
            src = self._state.manager.by_uuid(map_[src_node_uuid])
            dst = self._state.manager.by_uuid(map_[dst_node_uuid])

            log.debug('{} -> {} ({} -> {})'.format(src_node_uuid,
                        dst_node_uuid, src.device_name, dst.device_name))

            # XXX This should be moved to the various faces, that register to a
            # factory
            if protocol == FaceProtocol.ether:
                src_face = L2Face(node        = src_node,
                                  owner       = self,
                                  protocol    = protocol,
                                  src_nic     = src,
                                  dst_mac     = dst.mac_address)
                dst_face = L2Face(node        = dst_node,
                                  owner       = self,
                                  protocol    = protocol,
                                  src_nic     = dst,
                                  dst_mac     = src.mac_address)

            elif protocol in (FaceProtocol.tcp4, FaceProtocol.tcp6,
                    FaceProtocol.udp4, FaceProtocol.udp6):
                src_face = L4Face(node        = src_node,
                                  owner       = self,
                                  protocol    = protocol,
                                  src_ip      = src.ip4_address,
                                  dst_ip      = dst.ip4_address,
                                  src_port    = TMP_DEFAULT_PORT,
                                  dst_port    = TMP_DEFAULT_PORT)
                dst_face = L4Face(node        = dst_node,
                                  owner       = self,
                                  protocol    = protocol,
                                  src_ip      = dst.ip4_address,
                                  dst_ip      = src.ip4_address,
                                  src_port    = TMP_DEFAULT_PORT,
                                  dst_port    = TMP_DEFAULT_PORT)
            else:
                raise NotImplementedError

            # We key the sibling face for easier building of the ICN graph
            src_face._internal_data['sibling_face'] = dst_face._state.uuid
            dst_face._internal_data['sibling_face'] = src_face._state.uuid

            faces.append(src_face)
            faces.append(dst_face)

        return faces

#------------------------------------------------------------------------------

class ICNRoutes(Resource):
    """
    Resource: ICNRoutes

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
            data = G.get_edge_data(src, dst)

            map_ = data['map_node_face']
            next_hop_face = map_[src]

            route = ICNRoute(node   = src,
                             owner  = self,
                             prefix = prefix,
                             face   = next_hop_face)
            routes.append(route)
        return routes

#------------------------------------------------------------------------------

class DnsServerEntry(Resource):
    """
    Resource: DnsServerEntry

    Setup of DNS resolver for LxcContainers

    Todo:
      - This should be merged into the LxcContainer resource
    """

    node = Attribute(String)
    ip_address = Attribute(String)
    interface_name = Attribute(String)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    def __create__(self):
        return BashTask(self.node, CMD_CONTAINER_SET_DNS,
                {'ip_dns': self.ip_address,
                 'interface_name': self.interface_name})

    def __delete__(self):
        raise NotImplementedError

#------------------------------------------------------------------------------

class CentralIP(Resource):
    """
    Resource: CentralIP

    Central IP management (main resource)
    """

    ip_routing_strategy = Attribute(String, description = 'IP routing strategy',
            default = 'pair') # spt, pair
    ip6_data_prefix = Attribute(String, description="Prefix for IPv6 forwarding",
            mandatory = True)
    ip4_data_prefix = Attribute(String, description="Prefix for IPv4 forwarding",
            mandatory = True)
    ip6_max_link_prefix = Attribute(Integer,
            description = 'Maximum prefix size assigned to each link',
            default = 64)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after_init__(self):
        return ('Node', 'Channel', 'Interface')

    def __after__(self):
        return ('EmulatedChannel')

    def __subresources__(self):
        ip4_assign = Ipv4Assignment(prefix = self.ip4_data_prefix,
                groups = Reference(self, 'groups'))
        ip6_assign = Ipv6Assignment(prefix = self.ip6_data_prefix,
                groups = Reference(self, 'groups'),
                max_prefix_size = self.ip6_max_link_prefix)
        ip_routes = IPRoutes(owner = self,
                groups = Reference(self, 'groups'),
                routing_strategy = self.ip_routing_strategy)

        return (ip4_assign | ip6_assign) > ip_routes

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

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    __delete__ = None
