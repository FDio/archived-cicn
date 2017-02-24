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

from vicn.core.attribute            import Attribute, Multiplicity
from vicn.core.exception            import ResourceNotFound
from vicn.core.resource             import Resource
from vicn.core.task                 import EmptyTask, BashTask
from vicn.resource.ip.route         import IPRoute
from vicn.resource.node             import Node
from vicn.resource.vpp.vpp_commands import CMD_VPP_ADD_ROUTE
from vicn.resource.vpp.vpp_commands import CMD_VPP_ADD_ROUTE_GW

CMD_ADD_ROUTE = ('ip route add {route.ip_address} '
        'dev {route.interface.device_name} || true')
CMD_ADD_ROUTE_GW = ('ip route add {route.ip_address} '
        'dev {route.interface.device_name} via {route.gateway} || true')
CMD_DEL_ROUTE = ('ip route del {route.ip_address} '
        'dev {route.interface.device_name}')
CMD_SHOW_ROUTES = 'ip route show'

CMD_ADD_ARP_ENTRY = 'arp -s {route.ip_address} {route.mac_address}'

# Populate arp table too. The current configuration with one single bridge
# connecting every container and vpp nodes seem to create loops that prevent
# vpp from netmodel.network.interface for routing ip packets.

VPP_ARP_FIX = True

def _iter_routes(out):
    for line in out.splitlines():
        toks = line.strip().split()
        route = {'ip_address': toks[0]}
        for pos in range(1, len(toks)):
            if toks[pos] == '':
                pos+=1
            elif toks[pos] == 'dev':
                route['interface_name'] = toks[pos+1]
                pos+=2
            elif toks[pos] in ['src', 'proto', 'scope', 'metric']:
                pos+=2
            elif toks[pos] == 'via':
                route['gateway'] = toks[pos+1]
                pos+=2
            elif toks[pos] in ['linkdown', 'onlink']:
                pos+=1
        yield route

#------------------------------------------------------------------------------

class RoutingTable(Resource):
    """
    Resource: RoutingTable

    IP Routing Table management

    """

    node = Attribute(Node,
        mandatory = True,
        reverse_name = 'routing_table',
        reverse_description = 'Routing table of the node',
        reverse_auto = True,
        multiplicity = Multiplicity.OneToOne)

    routes = Attribute(IPRoute,
        multiplicity = Multiplicity.OneToMany)

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(self, *args, **kwargs)
        self._routes = dict()

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        return ('CentralIP', 'VPPInterface')

    def __get__(self):
        def cache(rv):
            for route in _iter_routes(rv.stdout):
                self._routes[route['ip_address']] = route

            # Force routing update
            raise ResourceNotFound
        return BashTask(self.node, CMD_SHOW_ROUTES, parse=cache)

    def __create__(self):
        """
        Create a single BashTask for all routes
        """

        done = set()
        routes_cmd = list()
        routes_via_cmd = list()
        arp_cmd = list()

        # vppctl lock
        # NOTE: we currently lock vppctl during the whole route update
        routes_lock = None
        routes_via_lock = None

        for route in self.routes:
            if route.ip_address in self._routes:
                continue
            if route.ip_address in done:
                continue
            done.add(route.ip_address)
            
            # TODO VPP should provide its own implementation of routing table
            # on the node
            if not route.interface.has_vpp_child: 
                if route.gateway is None:
                    cmd = CMD_ADD_ROUTE.format(route = route)
                    routes_cmd.append(cmd)
                else:
                    cmd = CMD_ADD_ROUTE_GW.format(route = route)
                    routes_via_cmd.append(cmd)
                if VPP_ARP_FIX and route.mac_address:
                    if route.ip_address != "default":
                        cmd = CMD_ADD_ARP_ENTRY.format(route = route)
                        arp_cmd.append(cmd)
            else:
                if route.gateway is None:
                    cmd = CMD_VPP_ADD_ROUTE.format(route = route)
                    routes_cmd.append(cmd)
                    routes_lock = route.node.vpp.vppctl_lock
                else:
                    cmd = CMD_VPP_ADD_ROUTE_GW.format(route = route)
                    routes_via_cmd.append(cmd)
                    routes_via_lock = route.node.vpp.vppctl_lock
                
        # TODO: checks
        clean_routes_task = EmptyTask()

        if len(routes_cmd) > 0:
            routes_task = BashTask(self.node, '\n'.join(routes_cmd),
                    lock = routes_lock)
        else:
            routes_task = EmptyTask()

        if len(routes_via_cmd) > 0:
            routes_via_task = BashTask(self.node, '\n'.join(routes_via_cmd),
                    lock = routes_via_lock)
        else:
            routes_via_task = EmptyTask()

        if len(arp_cmd) > 0:
            arp_task = BashTask(self.node, '\n'.join(arp_cmd))
        else:
            arp_task = EmptyTask()

        return ((clean_routes_task > routes_task) > routes_via_task) > arp_task

    def __delete__(self):
        raise NotImplementedError
