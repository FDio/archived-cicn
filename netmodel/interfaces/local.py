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

from netmodel.model.attribute   import Attribute
from netmodel.model.query       import Query, ACTION_INSERT
from netmodel.model.object      import Object
from netmodel.model.type        import String
from netmodel.network.interface import Interface, InterfaceState
from netmodel.network.packet    import Packet
from netmodel.network.prefix    import Prefix
from netmodel.util.misc         import lookahead

class LocalObjectInterface(Object):
    __type__ = 'local/interface'

    name = Attribute(String)
    type = Attribute(String)
    status = Attribute(String)
    description = Attribute(String)

    @classmethod
    def get(cls, query, ingress_interface):
        cb = ingress_interface._callback
        interfaces = ingress_interface._router.get_interfaces()
        for interface, last in lookahead(interfaces):
            interface_dict = {
                'name': interface.name,
                'type': interface.__interface__,
                'status': interface.get_status(),
                'description': interface.get_description(),
            }
            reply = Query(ACTION_INSERT, query.object_name, params =
                    interface_dict)
            reply.last = last
            packet = Packet.from_query(reply, reply = True)
            cb(packet, ingress_interface = ingress_interface)

class LocalInterface(Interface):
    __interface__ = 'local'

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._router = kwargs.pop('router')
        self.register_object(LocalObjectInterface)

