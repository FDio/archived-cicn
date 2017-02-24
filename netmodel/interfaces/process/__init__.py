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

import shlex
import socket
import subprocess
import threading

from netmodel.network.interface       import Interface as BaseInterface
from netmodel.network.packet          import Packet
from netmodel.network.prefix          import Prefix
from netmodel.model.attribute         import Attribute
from netmodel.model.filter            import Filter
from netmodel.model.object            import Object
from netmodel.model.query             import Query, ACTION_UPDATE
from netmodel.model.query             import ACTION_SUBSCRIBE, FUNCTION_SUM
from netmodel.model.type              import String, Integer, Double

DEFAULT_INTERVAL = 1 # s
KEY_FIELD = 'device_name'

class Interface(Object):
    __type__ = 'interface'

    node = Attribute(String)
    device_name = Attribute(String)
    bw_upstream = Attribute(Double) # bytes
    bw_downstream = Attribute(Double) # bytes

class Process(threading.Thread):
    pass

class BWMThread(Process):

    SEP=';'
    CMD="stdbuf -oL bwm-ng -t 1000 -N -o csv -c 0 -C '%s'" 

    # Parsing information (from README, specs section)
    # https://github.com/jgjl/bwm-ng/blob/master/README
    #
    # Type rate:
    FIELDS_RATE = ['timestamp', 'iface_name', 'bytes_out_s', 'bytes_in_s', 
            'bytes_total_s', 'bytes_in', 'bytes_out', 'packets_out_s',
            'packets_in_s', 'packets_total_s', 'packets_in', 'packets_out',
            'errors_out_s', 'errors_in_s', 'errors_in', 'errors_out']
    # Type svg, sum, max
    FIELDS_SUM  = ['timestamp', 'iface_name', 'bytes_out', 'bytes_in', 
            'bytes_total', 'packets_out', 'packets_in', 'packets_total', 
            'errors_out', 'errors_in']

    def __init__(self, interfaces, callback):
        threading.Thread.__init__(self)

        # The list of interfaces is used for filtering
        self.groups_of_interfaces = set(interfaces)

        self._callback = callback
        self._is_running = False

    def run(self):
        cmd = self.CMD % (self.SEP)
        p = subprocess.Popen(shlex.split(cmd), stdout = subprocess.PIPE, 
                stderr = subprocess.STDOUT)
        stdout = []
        self._is_running = True
        self.bwm_stats = dict()
        while self._is_running:
            line = p.stdout.readline().decode()
            if line == '' and p.poll() is not None:
                break
            if line:
                record = self._parse_line(line.strip())
                # We use 'total' to push the statistics back to VICN 
                if record['iface_name'] == 'total':
                    for interfaces in self.groups_of_interfaces:
                        if not len(interfaces) > 1:
                            # If the tuple contains only one interface, grab
                            # the information from bwm_stats and sends it back
                            # to VICN 
                            if interfaces[0] not in self.bwm_stats:
                                continue
                            interface = self.bwm_stats[interfaces[0]]
                            f_list = [[KEY_FIELD, '==', interface.device_name]]
                            query = Query(ACTION_UPDATE, Interface.__type__, 
                                    filter = Filter.from_list(f_list),
                                    params = interface.get_attribute_dict())
                            self._callback(query)
                        else:
                            # Iterate over each tuple of interfaces to create
                            # the aggregated filter and paramters to send back
                            # Currently, we only support sum among the stats
                            # when VICN subscribes to a tuple of interfaces
                            aggregated_filters = list()
                            aggregated_interface = Interface(
                                node          = socket.gethostname(),
                                device_name   = 'sum',
                                bw_upstream   = 0,
                                bw_downstream = 0)
                            predicate = list()
                            predicate.append(KEY_FIELD)
                            predicate.append('INCLUDED')
                            for interface in interfaces:
                                if interface not in self.bwm_stats:
                                    continue
                                iface = self.bwm_stats[interface]
                                aggregated_filters.append(iface.device_name)
                                aggregated_interface.bw_upstream += \
                                        iface.bw_upstream
                                aggregated_interface.bw_downstream += \
                                        iface.bw_downstream

                            if not aggregated_filters:
                                continue
                            predicate.append(aggregated_filters)

                            # We support mulitple interfaces only if tied up
                            # with the SUM function. The update must have the
                            # sum function specified because it is used to
                            # match the subscribe query
                            attrs = aggregated_interface.get_attribute_dict()
                            query = Query(ACTION_UPDATE, Interface.__type__, 
                                    filter = Filter.from_list([predicate]),
                                    params = attrs,
                                    aggregate = FUNCTION_SUM)
                            self._callback(query)
                else:
                    # Statistics from netmodel.network.interface will be stored
                    # in self.bwm_stats and used later to construct the update
                    # queries
                    interface = Interface(
                        node          = socket.gethostname(),
                        device_name   = record['iface_name'],
                        bw_upstream   = float(record['bytes_out_s']),
                        bw_downstream = float(record['bytes_in_s']),
                    )
                    
                    self.bwm_stats[record['iface_name']] = interface

        rc = p.poll()
        return rc

    def stop(self):
        self._is_running = False

    def _parse_line(self, line):
        return dict(zip(self.FIELDS_RATE, line.split(self.SEP)))

class BWMInterface(BaseInterface):
    __interface__ = 'bwm'

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._thread = None

        self.register_object(Interface)

    def terminate(self):
        self._thread.stop()

    def _on_reply(self, reply):
        packet = Packet.from_query(reply, reply = True)
        self.receive(packet)

    #-------------------------------------------------------------------------- 
    # Router interface
    #-------------------------------------------------------------------------- 

    def send_impl(self, packet):
        query = packet.to_query()

        assert query.action == ACTION_SUBSCRIBE
        interval = query.params.get('interval', DEFAULT_INTERVAL) \
                   if query.params else DEFAULT_INTERVAL
        assert interval

        # TODO: Add the sum operator. If sum the list of interfaces is
        # added to the BWMThread as a tuple, otherwise every single
        # interface will be added singularly

        # We currently simply extract it from the filter
        interfaces_list = [p.value for p in query.filter if p.key == KEY_FIELD]
        
        # interfaces is a list of tuple. If someone sbscribe to mulitple
        # interfaces interfaces will be a list of 1 tuple. The tuple will
        # contain the set of interfaces
        assert len(interfaces_list) == 1
        interfaces = interfaces_list[0] \
                     if isinstance(interfaces_list[0], tuple) \
                     else tuple([interfaces_list[0]])
        
        # Check if interfaces is more than one. In this case, we only support
        # The SUM function on the list of field.
        if len(interfaces) > 1:
            assert query.aggregate == FUNCTION_SUM

        if self._thread is None:
            self._thread = BWMThread(tuple([interfaces]), self._on_reply)
            self._thread.start()
        else:
            self._thread.groups_of_interfaces.add(interfaces)
