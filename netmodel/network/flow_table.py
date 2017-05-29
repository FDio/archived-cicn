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

from netmodel.model.query import ACTION_SUBSCRIBE, ACTION_UNSUBSCRIBE

# Per-interface flow table
class SubFlowTable:
    def __init__(self, interface):
        # Interface to which this flow table is associated
        self._interface = interface

        # Flow -> ingress interface
        self._flows = dict()

        # Ingress interface -> list of subscription
        self._subscriptions = dict()

    def add(self, packet, ingress_interface):
        flow = packet.get_flow()
        self._flows[flow] = ingress_interface

    def match(self, packet):
        flow = packet.get_flow()
        return self._flows.get(flow.get_reverse())

class Subscription:
    def __init__(self, packet, ingress_list, egress_list):
        """
        Args:
            packet : subscription packet
            ingress_list (List[Interface]) : list of ingress interface
            egress_list (List[Interface]) : list of egress interface
        """
        self._packet = packet
        self._ingress_list = ingress_list
        self._egress_list = egress_list

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

        # ingress_interface -> list of subscriptions
        self._ingress_subscriptions = dict()

        # egress_interface -> list of subscriptions
        self._egress_subscriptions = dict()


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
        
    def add(self, packet, ingress_interface, interface):
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
            s = Subscription(packet, [ingress_interface], [interface])

            if ingress_interface:
                if not ingress_interface in self._ingress_subscriptions:
                    self._ingress_subscriptions[ingress_interface] = list()
                self._ingress_subscriptions[ingress_interface].append(s)

            if not interface in self._egress_subscriptions:
                self._egress_subscriptions[interface] = list()
            self._egress_subscriptions[interface].append(s)

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
            del self._ingress_subscriptions[interface]
