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

from socket import inet_pton, inet_ntop, AF_INET6
from struct import unpack, pack
from abc    import ABCMeta

from netmodel.util.meta import inheritors

class BaseType:
    __choices__ = None

    @staticmethod
    def name():
        return self.__class__.__name__.lower()

    @classmethod
    def restrict(cls, **kwargs):
        class BaseType(cls):
            __choices__ = kwargs.pop('choices', None)
        return BaseType

class String(BaseType):
    __min_size__ = None
    __max_size__ = None
    __ascii__ = None
    __forbidden__ = None

    @classmethod
    def restrict(cls, **kwargs):
        base = super().restrict(**kwargs)
        class String(base):
            __max_size__ = kwargs.pop('max_size', None)
            __min_size__ = kwargs.pop('min_size', None)
            __ascii__ = kwargs.pop('ascii', None)
            __forbidden__ = kwargs.pop('forbidden', None)
        return String

class Integer(BaseType):
    def __init__(self, *args, **kwargs):
        self._min_value = kwargs.pop('min_value', None)
        self._max_value = kwargs.pop('max_value', None)
        super().__init__()

class Double(BaseType):
    def __init__(self, *args, **kwargs):
        self._min_value = kwargs.pop('min_value', None)
        self._max_value = kwargs.pop('max_value', None)
        super().__init__()

class Bool(BaseType):
    pass

class Dict(BaseType):
    pass

class PrefixTreeException(Exception): pass
class NotEnoughAddresses(PrefixTreeException): pass
class UnassignablePrefix(PrefixTreeException): pass

class Prefix(BaseType, metaclass=ABCMeta):

    def __init__(self, ip_address, prefix_len=None):
        if not prefix_len:
            if not isinstance(ip_address, str):
                import pdb; pdb.set_trace()
            if '/' in ip_address:
                ip_address, prefix_len = ip_address.split('/')
                prefix_len = int(prefix_len)
            else:
                prefix_len = self.MAX_PREFIX_SIZE
        if isinstance(ip_address, str):
            ip_address = self.aton(ip_address)
        self.ip_address = ip_address
        self.prefix_len = prefix_len

    def __contains__(self, obj):
        #it can be an IP as a integer
        if isinstance(obj, int):
            obj = type(self)(obj, self.MAX_PREFIX_SIZE)
        #Or it's an IP string
        if isinstance(obj, str):
            #It's a prefix as 'IP/prefix'
            if '/' in obj:
                split_obj = obj.split('/')
                obj = type(self)(split_obj[0], int(split_obj[1]))
            else:
                obj = type(self)(obj, self.MAX_PREFIX_SIZE)

        return self._contains_prefix(obj)

    @classmethod
    def mask(cls):
        mask_len = cls.MAX_PREFIX_SIZE//8 #Converts from bits to bytes
        mask = 0
        for step in range(0,mask_len):
            mask = (mask << 8) | 0xff
        return mask

    def _contains_prefix(self, prefix):
        assert isinstance(prefix, type(self))
        return (prefix.prefix_len >= self.prefix_len and
            prefix.ip_address >= self.first_prefix_address() and
            prefix.ip_address <= self.last_prefix_address())

    #Returns the first address of a prefix
    def first_prefix_address(self):
        return self.ip_address & (self.mask() << (self.MAX_PREFIX_SIZE-self.prefix_len))

    def canonical_prefix(self):
        return type(self)(self.first_prefix_address(), self.prefix_len)

    def last_prefix_address(self):
        return self.ip_address | (self.mask() >> self.prefix_len)

    def limits(self):
        return self.first_prefix_address(), self.last_prefix_address()

    def __str__(self):
        return "{}/{}".format(self.ntoa(self.first_prefix_address()), self.prefix_len)

    def __eq__(self, other):
        if not isinstance(other, self.__class__):
            return False
        return self.get_tuple() == other.get_tuple()

    def get_tuple(self):
        return (self.first_prefix_address(), self.prefix_len)

    def __hash__(self):
        return hash(self.get_tuple())

    def __iter__(self):
        return self.get_iterator()

    #Iterates by steps of prefix_len, e.g., on all available /31 in a /24
    def get_iterator(self, prefix_len=None, skip_internet_address=False):
        if prefix_len is None:
            prefix_len=self.MAX_PREFIX_SIZE
        assert (prefix_len >= self.prefix_len and prefix_len<=self.MAX_PREFIX_SIZE)
        step = 2**(self.MAX_PREFIX_SIZE - prefix_len)

        start = self.first_prefix_address()
        if skip_internet_address:
            start = start+1
        for ip in range(start, self.last_prefix_address()+1, step):
            yield type(self)(ip, prefix_len)

class Inet4Prefix(Prefix):

    MAX_PREFIX_SIZE = 32

    @classmethod
    def aton(cls, address):
        ret = 0
        components = address.split('.')
        for comp in components:
            ret = (ret << 8) + int(comp)
        return ret

    @classmethod
    def ntoa(cls, address):
        components = []
        for _ in range(0,4):
            components.insert(0,'{}'.format(address % 256))
            address = address >> 8
        return '.'.join(components)

class Inet6Prefix(Prefix):

    MAX_PREFIX_SIZE = 128

    @classmethod
    def aton (cls, address):
        prefix, suffix = unpack(">QQ", inet_pton(AF_INET6, address))
        return (prefix << 64) | suffix

    @classmethod
    def ntoa (cls, address):
        return inet_ntop(AF_INET6, pack(">QQ", address >> 64, address & ((1 << 64) -1)))

class InetAddress(Prefix):

    def get_tuple(self):
        return (self.ip_address, self.prefix_len)

    def __str__(self):
        return self.ntoa(self.ip_address)

class Inet4Address(InetAddress, Inet4Prefix):
    pass

class Inet6Address(InetAddress, Inet6Prefix):
    pass

class Self(BaseType):
    """Self-reference
    """

class Type:
    BASE_TYPES = (String, Integer, Double, Bool)
    _registry = dict()

    @staticmethod
    def from_string(type_name, raise_exception=True):
        """Returns a type corresponding to the type name.

        Params:
            type_name (str) : Name of the type

        Returns
            Type : Type class of the requested type name
        """
        type_cls = [t for t in Type.BASE_TYPES if t.name == type_name]
        if type_cls:
            return type_cls[0]

        type_cls = Type._registry.get(type_name, None)
        if not type_cls:
            raise Exception("No type found: {}".format(type_name))
        return type_cls

    @staticmethod
    def is_base_type(type_cls):
        return type_cls in Type.BASE_TYPES

    @staticmethod
    def exists(typ):
        return (isinstance(typ, type) and typ in inheritors(BaseType)) \
            or isinstance(typ, BaseType)

is_base_type = Type.is_base_type
is_type = Type.exists
