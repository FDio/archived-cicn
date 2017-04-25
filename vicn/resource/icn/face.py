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

from enum                       import Enum

from netmodel.model.type        import Integer, String, Bool
from vicn.core.attribute        import Attribute
from vicn.core.requirement      import Requirement
from vicn.core.resource         import Resource
from vicn.resource.node         import Node
from vicn.resource.interface    import Interface

DEFAULT_ETHER_PROTO = 0x0801
FMT_L4FACE = '{protocol.name}://{dst_ip}:{dst_port}/'
FMT_L2FACE = '{protocol.name}://[{dst_mac}]/{src_nic.device_name}'

class FaceProtocol(Enum):
    ether = 0
    ip4 = 1
    ip6 = 2
    tcp4 = 3
    tcp6 = 4
    udp4 = 5
    udp6 = 7
    app = 8

    @staticmethod
    def from_string(protocol):
        return getattr(FaceProtocol, protocol)

#------------------------------------------------------------------------------

class Face(Resource):
    """
    Resource: Face
    """

    node = Attribute(Node, mandatory = True,
            requirements = [
                Requirement('forwarder')
            ])
    protocol = Attribute(String,
            description = 'Face underlying protocol',
            mandatory = True)
    id = Attribute(String, description = 'Local face ID',
            ro = True)

    # Cisco's extensions
    wldr = Attribute(Bool, description = 'flag: WLDR enabled',
            default = False)
    x2 = Attribute(Bool, description = 'flag: X2 face',
            default = False)

    # NFD extensions
    permanent = Attribute(Bool, description = 'flag: permanent face',
            default = True)
    nfd_uri = Attribute(String, description = 'Face uri',
            func = lambda self : self._lambda_nfd_uri())
    nfdc_flags = Attribute(String,
            description = 'Flags for face creation with NFDC',
            func = lambda self : self._lambda_nfdc_flags())

    def __repr__(self):
        flags = ''
        if self.permanent:
            flags += 'permanent '
        if self.wldr:
            flags += 'wldr '
        if self.x2:
            flags += 'x2 '
        sibling_face_name = self._internal_data.get('sibling_face', None)
        sibling_face = self._state.manager.by_name(sibling_face_name) \
                if sibling_face_name else None
        dst_node = sibling_face.node.name if sibling_face else None
        return '<Face {} {} on node {} -- to node {}>'.format(
                self.nfd_uri, flags, self.node.name, dst_node)

    __str__ = __repr__

    # NFD specifics

    def _lambda_nfd_uri(self):
        raise NotImplementedError

    def _lambda_nfdc_flags(self):
        flags = ''
        if self.permanent:
            flags += '-P '
        if self.wldr:
            flags += '-W '
        if self.x2:
            flags += '-X '
        return flags

#------------------------------------------------------------------------------

class L2Face(Face):

    src_nic = Attribute(Interface,
            description = "Name of the network interface linked to the face",
            mandatory=True)
    dst_mac = Attribute(String, description = "destination MAC address",
            mandatory=True)
    ether_proto = Attribute(String,
            description = "Ethernet protocol number used by the face",
            default=DEFAULT_ETHER_PROTO)

    def _lambda_nfd_uri(self):
        return self.format(FMT_L2FACE)

#------------------------------------------------------------------------------

class L4Face(Face):

    ip_version = Attribute(Integer, description = "IPv4 or IPv6", default = 4)
    src_ip = Attribute(String, description = "local IP address",
            mandatory = True)
    src_port = Attribute(Integer, description = "local TCP/UDP port")
    dst_ip = Attribute(String, descrition = "remote IP address",
            mandatory=True)
    dst_port = Attribute(Integer, description = "remote TCP/UDP port",
            mandatory=True)

    def _lambda_nfd_uri(self):
        return self.format(FMT_L4FACE)
