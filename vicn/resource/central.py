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

import networkx as nx
import logging

from netmodel.model.type                import String
from netmodel.util.misc                 import pairwise
from vicn.core.address_mgr              import AddressManager
from vicn.core.attribute                import Attribute
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import Resource
from vicn.core.task                     import async_task, inline_task
from vicn.core.task                     import EmptyTask, BashTask
from vicn.resource.channel              import Channel
from vicn.resource.ip.route             import IPRoute
from vicn.resource.icn.forwarder        import Forwarder
from vicn.resource.icn.face             import L2Face, L4Face, FaceProtocol
from vicn.resource.icn.producer         import Producer
from vicn.resource.icn.route            import Route as ICNRoute
from vicn.resource.linux.file           import TextFile
from vicn.resource.lxd.lxc_container    import LxcContainer
from vicn.resource.node                 import Node

log = logging.getLogger(__name__)

TMP_DEFAULT_PORT = 6363

CMD_CONTAINER_SET_DNS = 'echo "nameserver {ip_dns}" | ' \
                        'resolvconf -a {interface_name}'

# For host
CMD_NAT = '\n'.join([
    'iptables -t nat -A POSTROUTING -o {bridge_name} -s {network} ' \
    '! -d {network} -j MASQUERADE',
    'echo 1 > /proc/sys/net/ipv4/ip_forward'
])

# For containers
CMD_IP_FORWARD = 'echo 1 > /proc/sys/net/ipv4/ip_forward'

HOST_FILE_PATH = "/etc/hosts.vicn"

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

def _get_l2_graph(manager, with_managed = False):
    G = nx.Graph()
    for node in manager.by_type(Node):
        G.add_node(node._state.uuid)

    for channel in manager.by_type(Channel):
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
            for src_it in range(0,len(channel.interfaces)):
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

def _get_icn_graph(manager):
    G = nx.Graph()
    for forwarder in manager.by_type(Forwarder):
        node = forwarder.node
        G.add_node(node._state.uuid)
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

class IPAssignment(Resource):
    """
    Resource: IPAssignment
    """

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        return ('Interface',)

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    def __subresources__(self):
        self.host_file = TextFile(node = None, filename = HOST_FILE_PATH,
                overwrite = True)
        return self.host_file

    def __create__(self):
        """
        IP assignment strategy /32: assign /32 IP address to each interface

        We might use different subnets for resources involved in experiment,
        and supporting resources, and to minimize routing tables.
        """
        log.info('Assigment of IP addresses')
        tasks = EmptyTask()

        # We sort nodes by names for IP assignment. This code ensures that
        # interfaces on the same channel get consecutive IP addresses. That
        # way, we can assign /31 on p2p channels.
        channels = sorted(self._state.manager.by_type(Channel), 
                key = lambda x : x.get_sortable_name())
        channels.extend(sorted(self._state.manager.by_type(Node), 
                    key = lambda node : node.name))

        host_file_content = ""
       
        # Dummy code to start IP addressing on an even number for /31
        ip = AddressManager().get_ip(None)
        if int(ip[-1]) % 2 == 0:
            ip = AddressManager().get_ip("dummy object")

        for channel in channels:
            # Sort interfaces in a deterministic order to ensure consistent
            # addressing across restarts of the tool
            interfaces = sorted(channel.interfaces, 
                    key = lambda x : x.device_name)

            for interface in interfaces:
                if interface.ip_address is None:
                    ip = AddressManager().get_ip(interface)

                    @async_task
                    async def set_ip(interface, ip):
                        await interface.async_set('ip_address', ip)

                    if interface.managed:
                        tasks = tasks | set_ip(interface, ip)
                else:
                    ip = interface.ip_address

                # Note: interface.ip_address should still be None at this stage
                # since we have not made the assignment yet

                if isinstance(channel, Node):
                    host_file_content += '# {} {} {}\n'.format(
                            interface.node.name, interface.device_name, ip)
                    if interface == interface.node.host_interface:
                        host_file_content += '{} {}\n'.format(ip, 
                                interface.node.name)
        self.host_file.content = host_file_content

        return tasks

    __delete__ = None

#-------------------------------------------------------------------------------

class IPRoutes(Resource):
    """
    Resource: IPRoutes

    Centralized IP route computation.
    """
    routing_strategy = Attribute(String)

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
        for node in self._state.manager.by_type(Node):
            node_uuid = node._state.uuid
            if not node_uuid in origins:
                origins[node_uuid] = list()
            for interface in node.interfaces:
                origins[node_uuid].append(interface.ip_address)
        return origins

    def _get_ip_routes(self):
        if self.routing_strategy == 'pair':
            return [], self._get_pair_ip_routes()

        strategy = MAP_ROUTING_STRATEGY.get(self.routing_strategy)

        G = _get_l2_graph(self._state.manager)
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
            
            if prefix == next_hop_ingress.ip_address:
                # Direct route on src_node.name :
                # route add [prefix] dev [next_hop_interface_.device_name]
                route = IPRoute(node     = src_node, 
                                managed    = False,
                                owner      = self,
                                ip_address = prefix,
                                mac_address = mac_addr, 
                                interface  = next_hop_interface)
            else:
                # We need to be sure we have a route to the gw from the node
                if not next_hop_ingress.ip_address in ip_routes[src_node]:
                    pre_route = IPRoute(node = src_node,
                                    managed    = False,
                                    owner      = self,
                                    ip_address = next_hop_ingress.ip_address,
                                    mac_address = mac_addr,
                                    interface  = next_hop_interface)
                    ip_routes[src_node].append(next_hop_ingress.ip_address)
                    pre_routes.append(pre_route)
                    
                # Route on src_node.name: 
                # route add [prefix] dev [next_hop_interface_.device_name]
                #    via [next_hop_ingress.ip_address]
                route = IPRoute(node     = src_node, 
                                managed    = False,
                                owner      = self,
                                ip_address = prefix,
                                interface  = next_hop_interface,
                                mac_address = mac_addr,
                                gateway    = next_hop_ingress.ip_address)
            ip_routes[src_node].append(prefix)
            routes.append(route)
        return pre_routes, routes

    def _get_pair_ip_routes(self):
        """
        IP routing strategy : direct routes only
        """
        routes = list()
        G = _get_l2_graph(self._state.manager) 
        for src_node_uuid, dst_node_uuid, data in G.edges_iter(data = True):
            src_node = self._state.manager.by_uuid(src_node_uuid)
            dst_node = self._state.manager.by_uuid(dst_node_uuid)

            map_ = data['map_node_interface']
            src = self._state.manager.by_uuid(map_[src_node_uuid])
            dst = self._state.manager.by_uuid(map_[dst_node_uuid])

            log.debug('[IP ROUTE] NODES {}/{}/{} -> {}/{}/{}'.format(
                        src_node.name, src.device_name, src.ip_address,
                        dst_node.name, dst.device_name, dst.ip_address))
            log.debug('[IP ROUTE] NODES {}/{}/{} -> {}/{}/{}'.format(
                        dst_node.name, dst.device_name, dst.ip_address,
                        src_node.name, src.device_name, src.ip_address))

            route = IPRoute(node        = src_node, 
                            managed     = False,
                            owner       = self,
                            ip_address  = dst.ip_address,
                            mac_address = dst.mac_address,
                            interface   = src)
            routes.append(route)

            route = IPRoute(node       = dst_node, 
                            managed    = False,
                            owner      = self,
                            ip_address = src.ip_address,
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
        G = _get_l2_graph(self._state.manager)
        for src_node_uuid, dst_node_uuid, data in G.edges_iter(data = True):
            src_node = self._state.manager.by_uuid(src_node_uuid)
            dst_node = self._state.manager.by_uuid(dst_node_uuid)

            map_ = data['map_node_interface']
            src = self._state.manager.by_uuid(map_[src_node_uuid])
            dst = self._state.manager.by_uuid(map_[dst_node_uuid])

            log.debug('{} -> {} ({} -> {})'.format(src_node_uuid,
                        dst_node_uuid, src.device_name, dst.device_name))

            if protocol == FaceProtocol.ether:
                src_face = L2Face(node        = src_node,
                                  owner      = self,
                                  protocol    = protocol,
                                  src_nic     = src,
                                  dst_mac     = dst.mac_address) 
                dst_face = L2Face(node        = dst_node,
                                  owner      = self,
                                  protocol    = protocol,
                                  src_nic     = dst,
                                  dst_mac     = src.mac_address)

            elif protocol in (FaceProtocol.tcp4, FaceProtocol.tcp6, 
                    FaceProtocol.udp4, FaceProtocol.udp6):
                src_face = L4Face(node        = src_node,
                                  owner      = self,
                                  protocol    = protocol,
                                  src_ip      = src.ip_address,
                                  dst_ip      = dst.ip_address,
                                  src_port    = TMP_DEFAULT_PORT,
                                  dst_port    = TMP_DEFAULT_PORT) 
                dst_face = L4Face(node        = dst_node,
                                  owner      = self,
                                  protocol    = protocol,
                                  src_ip      = dst.ip_address,
                                  dst_ip      = src.ip_address,
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
        for producer in self._state.manager.by_type(Producer):
            node_uuid = producer.node._state.uuid
            if not node_uuid in origins:
                origins[node_uuid] = list()
            origins[node_uuid].extend(producer.prefixes)
        return origins

    def _get_icn_routes(self):
        strategy = MAP_ROUTING_STRATEGY.get(self.routing_strategy)

        G = _get_icn_graph(self._state.manager)
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

class ContainerSetup(Resource):
    """
    Resource: ContainerSetup

    Setup of container networking

    Todo:
      - This should be merged into the LxcContainer resource
    """

    container = Attribute(LxcContainer)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __subresources__(self):

        # a) routes: host -> container
        #   . container interfaces
        #   . container host (main) interface
        # route add -host {ip_address} dev {bridge_name}
        route = IPRoute(node       = self.container.node,
                        managed    = False,
                        owner      = self,
                        ip_address = self.container.host_interface.ip_address, 
                        interface  = self.container.node.bridge)
        route.node.routing_table.routes << route

        # b) route: container -> host
        # route add {ip_gateway} dev {interface_name}
        # route add default gw {ip_gateway} dev {interface_name}
        route = IPRoute(node       = self.container,
                        owner      = self,
                        managed    = False,
                        ip_address = self.container.node.bridge.ip_address,
                        interface  = self.container.host_interface)
        route.node.routing_table.routes << route
        route_gw = IPRoute(node       = self.container,
                           managed    = False,
                           owner      = self,
                           ip_address = 'default',
                           interface  = self.container.host_interface,
                           gateway    = self.container.node.bridge.ip_address)
        route_gw.node.routing_table.routes << route_gw

        # c) dns
        dns_server_entry = DnsServerEntry(node = self.container, 
                owner      = self,
                ip_address = self.container.node.bridge.ip_address, 
                interface_name = self.container.host_interface.device_name)

        return dns_server_entry

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    def __create__(self):
        return BashTask(self.container.node, CMD_IP_FORWARD)

#------------------------------------------------------------------------------

class ContainersSetup(Resource):
    """
    Resource: ContainersSetup

    Setup of LxcContainers (main resource)

    Todo:
      - This should be merged into the LxcContainer resource
    """

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __subresources__(self):
        containers  = self._state.manager.by_type(LxcContainer)
        if len(containers) == 0:
            return None

        container_resources = [ContainerSetup(owner = self, container = c) 
            for c in containers]

        return Resource.__concurrent__(*container_resources)

#------------------------------------------------------------------------------

class CentralIP(Resource):
    """
    Resource: CentralIP

    Central IP management (main resource)
    """

    ip_routing_strategy = Attribute(String, description = 'IP routing strategy',
            default = 'pair') # spt, pair

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after_init__(self):
        return ('Node', 'Channel', 'Interface')

    def __subresources__(self):
        ip_assign = IPAssignment(owner=self)
        containers_setup = ContainersSetup(owner=self)
        ip_routes = IPRoutes(owner = self, 
                routing_strategy = self.ip_routing_strategy)

        return ip_assign > (ip_routes | containers_setup)

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    __delete__ = None

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
        icn_faces = ICNFaces(owner = self, protocol_name = self.face_protocol) 
        icn_routes = ICNRoutes(owner = self, 
                routing_strategy = self.icn_routing_strategy)
        return icn_faces > icn_routes

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    __delete__ = None
