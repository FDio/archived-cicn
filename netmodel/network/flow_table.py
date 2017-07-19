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

import copy
from collections            import defaultdict, Counter

from netmodel.model.query   import Query, ACTION_SUBSCRIBE, ACTION_UNSUBSCRIBE

# Per-interface flow table
class SubFlowTable:
    def __init__(self, interface):
        # Interface to which this flow table is associated
        self._interface = interface

        # Flow -> ingress interface
        self._flows = defaultdict(set)

    def add(self, packet, ingress_interface):
        flow = packet.get_flow()
        self._flows[flow].add(ingress_interface)

    def match(self, packet):
        flow = packet.get_flow()
        return self._flows.get(flow.get_reverse())

    def _on_interface_delete(self, interface):
        """
        Returns:
            False is the flow table is empty.
        """
        to_remove = set()
        for flow, ingress_interfaces in self._flows.items():
            ingress_interfaces.discard(interface)
            if not ingress_interfaces:
                to_remove.add(flow)
        for flow in to_remove:
            del self._flows[flow]

        return len(self._flows) > 0

class Subscription:
    def __init__(self, packet, ingress_set, egress_set):
        """
        Args:
            packet : subscription packet
            ingress_list (List[Interface]) : list of ingress interface
            egress_list (List[Interface]) : list of egress interface
        """
        self._packet = packet
        #self._ingress_set = ingress_set
        #self._egress_set = egress_set

    def get_tuple(self):
        return (self._packet,)

    def __eq__(self, other):
        return self.get_tuple() == other.get_tuple()

    def __hash__(self):
        return hash(self.get_tuple())

    def __repr__(self):
        return '<Subscription {}>'.format(self._packet.to_query())

    __str__ = __repr__

class FlowTable:
    """
    The flow table managed flow association between packets, as well as the
    subscription state for each flow.

    Event management
    ================

    The flow table has to be notified from netmodel.network.interface becomes
    up, and when it is deleted. This is handled through the following events:

    _on_interface_up
        . resend any pending subscription on the interface
    _on_interface_deleted
        . delete any subscription
    """

    def __init__(self):
        # Per interface (sub) flow tables
        #  flow are added when forwarded using FIB
        #  matched upon returning
        self._sub_flow_tables = dict()

        # The Flow Table also maintains a list of subscriptions doubly indexed
        # by both subscriptors, and egress interfaces

        # ingress_interface -> bag of subscriptions
        self._ingress_subscriptions = defaultdict(Counter)

        # egress_interface -> bag of subscriptions
        self._egress_subscriptions = defaultdict(Counter)

    def dump(self, msg=''):
        print("="*80)
        print("FLOW TABLE {}".format(msg))
        print("-" * 80)
        print("SubFlowTables")
        for interface, flow_table in self._sub_flow_tables.items():
            for k, v in flow_table._flows.items():
                print(interface, "\t", k, "\t", v)
        print("-" * 80)
        print("Ingress subscriptions")
        for interface, subscriptions in self._ingress_subscriptions.items():
            print(interface, "\t", subscriptions)
        print("-" * 80)
        print("Egress subscriptions")
        for interface, subscriptions in self._egress_subscriptions.items():
            print(interface, "\t", subscriptions)
        print("=" * 80)
        print("")

    def match(self, packet, interface):
        """
        Check whether the packet arriving on interface is a reply.

        Returns:
            interface that originally requested the packet
            None if not found
        """
        sub_flow_table = self._sub_flow_tables.get(interface)
        if not sub_flow_table:
            return None
        return sub_flow_table.match(packet)

    def add(self, packet, ingress_interface, interfaces):
        for interface in interfaces:
            sub_flow_table = self._sub_flow_tables.get(interface)
            if not sub_flow_table:
                sub_flow_table = SubFlowTable(interface)
                self._sub_flow_tables[interface] = sub_flow_table
            sub_flow_table.add(packet, ingress_interface)

        # If the flow is a subscription, we need to associate it to the list
        query = packet.to_query()
        if query.action == ACTION_SUBSCRIBE:
            # XXX we currently don't merge subscriptions, and assume a single
            # next hop interface
            s = Subscription(packet, set([ingress_interface]), interfaces)

            if ingress_interface:
                self._ingress_subscriptions[ingress_interface] += Counter([s])
            for interface in interfaces:
                self._egress_subscriptions[interface] += Counter([s])

        elif query.action == ACTION_UNSUBSCRIBE:
            raise NotImplementedError

    # Events

    def _on_interface_up(self, interface):
        """
        Callback: an interface gets back up after downtime.

        Resend all pending subscriptions when an interface comes back up.
        """
        subscriptions = self._egress_subscriptions.get(interface)
        if not subscriptions:
            return
        for s in subscriptions:
            interface.send(s._packet)

    def _on_interface_delete(self, interface):
        """
        Callback: an interface has been deleted

        Cancel all subscriptions that have been issues from
            netmodel.network.interface.
        Remove all pointers to subscriptions pending on this interface
        """

        if interface in self._ingress_subscriptions:
            # If the interface we delete at the origin of the subscription,
            # let's also remove corresponding egress subscriptions
            subs = self._ingress_subscriptions[interface]
            if not subs:
                return

            to_remove = set()
            for _interface, subscriptions in self._egress_subscriptions.items():

                removed = subs & subscriptions
                if removed:
                    for s in removed:
                        # We found a subscription of this interface on an other
                        # interface; send unsubscribe...
                        action, params = s._packet.payload
                        p = copy.deepcopy(s._packet)
                        p.payload = (ACTION_UNSUBSCRIBE, params)
                        _interface.send(p)
                    # ... and remove them
                    subscriptions -= removed

                    # if the interface has no more subscription remove it.
                    if not subscriptions:
                        to_remove.add(_interface)

            for i in to_remove:
                del self._egress_subscriptions[i]

            del self._ingress_subscriptions[interface]

        # Remove interface from flow table destination
        to_remove = set()
        for _interface, sub_flow_table in self._sub_flow_tables.items():
            remove = sub_flow_table._on_interface_delete(interface)
            if not remove:
                to_remove.add(_interface)
        for _interface in to_remove:
            del self._sub_flow_tables[_interface]
