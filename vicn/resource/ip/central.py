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

from netmodel.model.type                import String, Integer, Bool
from vicn.core.attribute                import Attribute, Reference
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import Resource
from vicn.core.task                     import inline_task, BashTask
from vicn.resource.central              import _get_l2_graph, MAP_ROUTING_STRATEGY
from vicn.resource.channel              import Channel
from vicn.resource.ip_assignment        import Ipv4Assignment, Ipv6Assignment
from vicn.resource.ip.route             import IPRoute

log = logging.getLogger(__name__)

CMD_CONTAINER_SET_DNS = 'echo "nameserver {ip_dns}" | ' \
                        'resolvconf -a {interface_name}'

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
                    if group.name not in [x.name for x in interface.node.groups]:
                        continue
                    node_uuid = interface.node._state.uuid
                    if not node_uuid in origins:
                        origins[node_uuid] = list()
                    if interface.ip4_address:
                        origins[node_uuid].append(interface.ip4_address.canonical_prefix())
                    if interface.ip6_address:
                        origins[node_uuid].append(interface.ip6_address.canonical_prefix())
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

            # Avoid duplicate routes due to multiple paths in the network
            if not src_node in ip_routes:
                ip_routes[src_node] = set()
                #When you set up an IP address ip/prefix_len (ip addr add), you already create a route
                #towards ip/prefix_len
                for interface in src_node.interfaces:
                    ip_routes[src_node].add(interface.ip4_address.canonical_prefix())
                    ip_routes[src_node].add(interface.ip6_address.canonical_prefix())
            if prefix in ip_routes[src_node]:
                continue
            ip_routes[src_node].add(prefix)

            ip_version = 6 if ":" in str(prefix) else 4
            next_hop_ingress_ip = (next_hop_ingress.ip6_address if ip_version is 6 else
                    next_hop_ingress.ip4_address)
            if prefix == next_hop_ingress_ip:
                # Direct route on src_node.name :
                # route add [prefix] dev [next_hop_interface_.device_name]
                route = IPRoute(node     = src_node,
                                managed    = False,
                                owner      = self,
                                ip_address = prefix,
                                ip_version = ip_version,
                                interface  = next_hop_interface)
            else:
                # We need to be sure we have a route to the gw from the node
                if not next_hop_ingress_ip.canonical_prefix() in ip_routes[src_node]:
                    pre_route = IPRoute(node    = src_node,
                                    managed     = False,
                                    owner       = self,
                                    ip_address  = next_hop_ingress_ip.canonical_prefix(),
                                    ip_version  = ip_version,
                                    interface   = next_hop_interface)
                    ip_routes[src_node].add(next_hop_ingress_ip.canonical_prefix())
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
                                gateway     = next_hop_ingress_ip)
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
                {'ip_dns': str(self.ip_address),
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
    ip6_disjoint_addresses = Attribute(Bool, description="assign disjoint addresses spanning over the whole prefix", default=False)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after_init__(self):
        return ('Node', 'Channel', 'Interface', 'EmulatedChannel')

    def __subresources__(self):
        ip4_assign = Ipv4Assignment(prefix = self.ip4_data_prefix,
                groups = Reference(self, 'groups'))
        ip6_assign = Ipv6Assignment(prefix = self.ip6_data_prefix,
                groups = Reference(self, 'groups'),
                max_prefix_len = self.ip6_max_link_prefix,
                disjoint_addresses = self.ip6_disjoint_addresses)
        ip_routes = IPRoutes(owner = self,
                groups = Reference(self, 'groups'),
                routing_strategy = self.ip_routing_strategy)

        return (ip4_assign | ip6_assign) > ip_routes
