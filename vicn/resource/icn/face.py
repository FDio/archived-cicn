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

from netmodel.model.type            import Integer, String, Bool, InetAddress
from netmodel.model.object          import ObjectMetaclass
from vicn.core.attribute            import Attribute
from vicn.core.requirement          import Requirement
from vicn.core.resource             import Resource
from vicn.resource.interface        import Interface
from vicn.resource.node             import Node

DEFAULT_ETHER_PROTO = 0x0801
DEFAULT_PORT = 6363

FMT_L4FACE_IPV4 = '{protocol}://{dst.ip4_address}:{dst_port}/'
FMT_L4FACE_IPV6 = '{protocol}://{dst.ip6_address}:{dst_port}/'
FMT_L2FACE = '{protocol}://[{dst.mac_address}]/{src.device_name}'

#------------------------------------------------------------------------------

class FaceMetaclass(ObjectMetaclass):
    def __new__(mcls, name, bases, attrs):
        cls = super(FaceMetaclass, mcls).__new__(mcls, name, bases, attrs)
        if name != 'Face':
            cls.register()
        return cls

class Face(Resource, metaclass=FaceMetaclass):
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

    src = Attribute(Interface, mandatory = True)
    dst = Attribute(Interface, mandatory = True)

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

    # map protocol -> face class
    _map_protocol = dict()

    @classmethod
    def register(cls):
        for protocol in cls.__protocols__:
            Face._map_protocol[protocol] = cls

    @classmethod
    def from_protocol(cls, protocol):
        return cls._map_protocol.get(protocol)

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
        return 'N/A' # raise NotImplementedError

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

    __protocols__ = ['ether']

    ether_proto = Attribute(String,
            description = "Ethernet protocol number used by the face",
            default = DEFAULT_ETHER_PROTO)

    def _lambda_nfd_uri(self):
        return self.format(FMT_L2FACE)

#------------------------------------------------------------------------------

class L4Face(Face):

    __protocols__ = ['tcp4', 'tcp6', 'udp4', 'udp6']

    src_port = Attribute(Integer, description = "local TCP/UDP port",
            default = DEFAULT_PORT)
    dst_port = Attribute(Integer, description = "remote TCP/UDP port",
            default = DEFAULT_PORT)

    def _lambda_nfd_uri(self):
        fmt = FMT_L4FACE_IPV4 if self.protocol in ['tcp4', 'udp4'] else FMT_L4FACE_IPV6
        return self.format(fmt)
