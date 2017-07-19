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
import enum
import logging
import pickle
import traceback
import uuid

from types                          import GeneratorType

from netmodel.network.flow          import Flow
from netmodel.network.prefix        import Prefix as Prefix_
from netmodel.model.attribute       import Attribute
from netmodel.model.type            import String
from netmodel.model.query           import Query
from netmodel.model.filter          import Filter as Filter_
from netmodel.model.field_names     import FieldNames as FieldNames_
from netmodel.model.object          import Object

log = logging.getLogger(__name__)

class NetmodelException(Exception):
    pass

#------------------------------------------------------------------------------

NETMODEL_TLV_TYPELEN_STR = '!H'
NETMODEL_TLV_SIZE = 2
NETMODEL_TLV_TYPE_MASK = 0xfe00
NETMODEL_TLV_TYPE_SHIFT = 9
NETMODEL_TLV_LENGTH_MASK = 0x01ff

NETMODEL_TLV_OBJECT_NAME    = 1
NETMODEL_TLV_FIELD          = 2
NETMODEL_TLV_FIELDS         = 3
NETMODEL_TLV_PREDICATE      = 4
NETMODEL_TLV_FILTER         = 5
NETMODEL_TLV_SRC            = 6
NETMODEL_TLV_DST            = 7
NETMODEL_TLV_PROTOCOL       = 8
NETMODEL_TLV_FLAGS          = 9
NETMODEL_TLV_PAYLOAD        = 10
NETMODEL_TLV_PACKET         = 11

SUCCESS = 0
WARNING = 1
ERROR   = 2

FLAG_LAST   = 1 << 0
FLAG_REPLY  = 1 << 1

#------------------------------------------------------------------------------

class PacketProtocol(enum.Enum):
    Query = 'query'
    Error = 'error'

#------------------------------------------------------------------------------

class VICNTLV:

    _LEN_MIN = 0
    _LEN_MAX = 511
    tlv_type = None

    _tlv_parsers = {} # Attributes...
    tlvs = []

    def decode(self, buf):
        (self.typelen, ) = struct.unpack(
            NETMODEL_TLV_TYPELEN_STR, buf[:NETMODEL_TLV_SIZE])
        tlv_type = \
            (self.typelen & NETMODEL_TLV_TYPE_MASK) >> NETMODEL_TLV_TYPE_SHIFT
        assert self.tlv_type == tlv_type

        self.len = self.typelen & NETMODEL_TLV_LENGTH_MASK
        assert len(buf) >= self.len + NETMODEL_TLV_SIZE

        self.tlv_info = buf[NETMODEL_TLV_SIZE:]
        self.tlv_info = self.tlv_info[:self.len]

    #--------------------------------------------------------------------------
    # Descriptor protocol
    #--------------------------------------------------------------------------

    @classmethod
    def _get_tlv_parsers(cls):
        if not cls._tlv_parsers:
            cls._tlv_parsers = None
        return cls._tlv_parsers


    @staticmethod
    def get_type(buf):
        (typelen, ) = struct.unpack(NETMODEL_TLV_TYPELEN_STR,
                buf[:NETMODEL_TLV_SIZE])
        return (typelen & NETMODEL_TLV_TYPE_MASK) >> NETMODEL_TLV_TYPE_SHIFT


    def _len_valid(self):
        return self._LEN_MIN <= self.len and self.len <= self._LEN_MAX

    #--------------------------------------------------------------------------

    @classmethod
    def _parser(cls, buf):
        tlvs = []

        while buf:
            tlv_type = VICNTLV.get_type(buf)
            tlv = cls._tlv_parsers[tlv_type](buf)
            tlvs.append(tlv)
            offset = NETMODEL_TLV_SIZE + tlv.len
            buf = buf[offset:]
            if tlv.tlv_type == NETMODEL_TLV_END:
                break
            assert len(buf) > 0

        pkt = cls(tlvs)

        assert pkt._tlvs_len_valid()
        assert pkt._tlvs_valid()

        return pkt, None, buf

    @classmethod
    def parser(cls, buf):
        try:
            return cls._parser(buf)
        except:
            return None, None, buf

    def serialize(self, payload, prev):
        data = bytearray()
        for tlv in self.tlvs:
            data += tlv.serialize()

        return data

    @classmethod
    def set_type(cls, tlv_cls):
        cls._tlv_parsers[tlv_cls.tlv_type] = tlv_cls

    @classmethod
    def get_type(cls, tlv_type):
        return cls._tlv_parsers[tlv_type]

    @classmethod
    def set_tlv_type(cls, tlv_type):
        def _set_type(tlv_cls):
            tlv_cls.tlv_type = tlv_type
            #cls.set_type(tlv_cls)
            return tlv_cls
        return _set_type

    def __len__(self):
        return sum(NETMODEL_TLV_SIZE + tlv.len for tlv in self.tlvs)

#------------------------------------------------------------------------------

@VICNTLV.set_tlv_type(NETMODEL_TLV_OBJECT_NAME)
class ObjectName(VICNTLV): pass

@VICNTLV.set_tlv_type(NETMODEL_TLV_FIELD)
class Field(VICNTLV):
    """Field == STR
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_PREDICATE)
class Predicate(VICNTLV):
    """Predicate == key, op, value
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_FILTER)
class Filter(Filter_, VICNTLV):
    """Filter == Array<Predicates>
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_FIELDS)
class FieldNames(FieldNames_, VICNTLV):
    """Fields == Array<Field>
    """


class Prefix(Object, Prefix_, VICNTLV):
    object_name = ObjectName()
    filter      = Filter()
    field_names = FieldNames()

    def __init__(self, *args, **kwargs):
        Object.__init__(self)
        Prefix_.__init__(self, *args, **kwargs)
        VICNTLV.__init__(self)

    def get_tuple(self):
        return (self.object_name, self.filter, self.field_names)

    def __eq__(self, other):
        return self.get_tuple() == other.get_tuple()

    def __hash__(self):
        return hash(self.get_tuple())

@VICNTLV.set_tlv_type(NETMODEL_TLV_SRC)
class Source(Prefix):
    """Source address
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_DST)
class Destination(Prefix):
    """Destination address
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_PROTOCOL)
class Protocol(Attribute, VICNTLV):
    """Protocol
    """


@VICNTLV.set_tlv_type(NETMODEL_TLV_FLAGS)
class Flags(Attribute, VICNTLV):
    """Flags: last, ...
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_PAYLOAD)
class Payload(Attribute, VICNTLV):
    """Payload
    """

@VICNTLV.set_tlv_type(NETMODEL_TLV_PACKET)
class Packet(Object, VICNTLV):
    """Base packet class
    """
    source = Source()
    destination = Destination(Prefix)
    protocol = Protocol(String, default = 'query')
    flags = Flags(String)
    payload = Payload(String)

    # This should be dispatched across L3 L4 L7

    def __init__(self, source = None, destination = None, protocol = None,
            flags = 0, payload = None):
        self.source         = source
        self.destination    = destination
        self.protocol       = protocol
        self.flags          = flags
        self.payload        = payload

    def get_flow(self):
        return Flow(self.source, self.destination)

    @staticmethod
    def from_query(query, src_query = None, reply = False):
        packet = Packet()
        if src_query:
            address = Prefix(
                    object_name = src_query.object_name,
                    filter = src_query.filter,
                    field_names = src_query.field_names,
                    aggregate = src_query.aggregate)
            if reply:
                packet.destination = address
            else:
                packet.source = address

        if query:
            address = Prefix(
                    object_name = query.object_name,
                    filter = query.filter,
                    field_names = query.field_names,
                    aggregate = query.aggregate)

            if reply:
                packet.source = address
            else:
                packet.destination = address

            packet.payload = (query.action, query.params)

        packet.protocol = 'sync'
        packet.last = not query or query.last
        packet.reply = reply

        return packet

    def to_query(self):
        action, params = self.payload

        address = self.source if self.reply else self.destination
        object_name = address.object_name
        filter = address.filter
        field_names = address.field_names
        aggregate = address.aggregate

        return Query(action, object_name, filter, params, field_names,
                aggregate = aggregate, last = self.last, reply = self.reply)

    @property
    def last(self):
        return self.flags & FLAG_LAST

    @last.setter
    def last(self, last):
        if last:
            self.flags |= FLAG_LAST
        else:
            self.flags &= ~FLAG_LAST

    @property
    def reply(self):
        return self.flags & FLAG_REPLY

    @reply.setter
    def reply(self, reply):
        if reply:
            self.flags |= FLAG_REPLY
        else:
            self.flags &= ~FLAG_REPLY

    def get_tuple(self):
        return (self.source, self.destination, self.protocol, self.flags,
                self.payload)

    def __eq__(self, other):
        return self.get_tuple() == other.get_tuple()

    def __hash__(self):
        return hash(self.get_tuple())

class ErrorPacket(Packet):
    """
    Analog with ICMP errors packets in IP networks
    """

    #--------------------------------------------------------------------------
    # Constructor
    #--------------------------------------------------------------------------

    def __init__(self, type = ERROR, code = ERROR, message = None,
            traceback = None, **kwargs):
        assert not traceback or isinstance(traceback, str)

        Packet.__init__(self, **kwargs)
        self.protocol = PacketProtocol.Error
        self.last = True
        self._type      = type
        self._code      = code
        self._message   = message
        self._traceback = traceback

    #--------------------------------------------------------------------------
    # Static methods
    #--------------------------------------------------------------------------

    @staticmethod
    def from_exception(packet, e):
        if isinstance(e, NetmodelException):
            error_packet = ErrorPacket(
                type      = e.TYPE, # eg. ERROR
                code      = e.CODE, # eg. BADARGS
                message   = str(e), #e.message,
                traceback = traceback.format_exc(),
                last      = True
            )
        else:
            error_packet = ErrorPacket(
                type      = ERROR,
                code      = UNHANDLED_EXCEPTION,
                message   = str(e),
                traceback = traceback.format_exc(),
                last      = True
            )
        error_packet.set_source(packet.get_destination())
        error_packet.set_destination(packet.get_source())
        return error_packet

    def get_message(self):
        """
        Returns:
            The error message related to this ErrorPacket.
        """
        return self._message

    def get_traceback(self):
        """
        Returns:
            The traceback related to this ErrorPacket.
        """
        return self._traceback

    def get_origin(self):
        """
        Returns:
            A value among {code::CORE, code::GATEWAY}
            identifying who is the origin of this ErrorPacket.
        """
        return self._origin

    def get_code(self):
        """
        Returns:
            The error code of the Error carried by this ErrorPacket.
        """
        return self._code

    def get_type(self):
        """
        Returns:
            The error type of the Error carried by this ErrorPacket.
        """
        return self._type

    def __repr__(self):
        """
        Returns:
            The '%r' representation of this ERROR Packet.
        """
        return "<Packet.%s: %s>" % (
            Packet.get_protocol_name(self.get_protocol()),
            self.get_message()
        )
