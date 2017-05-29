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

class PrefixTreeException(Exception): pass
class NotEnoughAddresses(PrefixTreeException): pass
class UnassignablePrefix(PrefixTreeException): pass

class Prefix(metaclass=ABCMeta):

    def __init__(self, ip_address, prefix_size=None):
        if not prefix_size:
            ip_address, prefix_size = ip_address.split('/')
            prefix_size = int(prefix_size)
        if isinstance(ip_address, str):
            ip_address = self.aton(ip_address)
        self.ip_address = ip_address
        self.prefix_size = prefix_size

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
        return (prefix.prefix_size >= self.prefix_size and
            prefix.ip_address >= self.first_prefix_address() and
            prefix.ip_address <= self.last_prefix_address())

    #Returns the first address of a prefix
    def first_prefix_address(self):
        return self.ip_address & (self.mask() << (self.MAX_PREFIX_SIZE-self.prefix_size))

    def last_prefix_address(self):
        return self.ip_address | (self.mask() >> self.prefix_size)

    def limits(self):
        return self.first_prefix_address(), self.last_prefix_address()

    def __str__(self):
        return "{}/{}".format(self.ntoa(self.first_prefix_address()), self.prefix_size)

    def __eq__(self, other):
        return (self.first_prefix_address() == other.first_prefix_address() and
                self.prefix_size == other.prefix_size)

    def __hash__(self):
        return hash(str(self))

    def __iter__(self):
        return self.get_iterator()

    #Iterates by steps of prefix_size, e.g., on all available /31 in a /24
    def get_iterator(self, prefix_size=None):
        if prefix_size is None:
            prefix_size=self.MAX_PREFIX_SIZE
        assert (prefix_size >= self.prefix_size and prefix_size<=self.MAX_PREFIX_SIZE)
        step = 2**(self.MAX_PREFIX_SIZE - prefix_size)
        for ip in range(self.first_prefix_address(), self.last_prefix_address()+1, step):
            yield type(self)(ip, prefix_size)

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

    #skip_internet_address: skip a:b::0, as v6 often use default /64 prefixes
    def get_iterator(self, prefix_size=None, skip_internet_address=None):
        if skip_internet_address is None:
            #We skip the internet address if we iterate over Addresses
            if prefix_size is None:
                skip_internet_address = True
            #But not if we iterate over prefixes
            else:
                skip_internet_address = False
        it = super().get_iterator(prefix_size)
        if skip_internet_address:
            next(it)
        return it

###### PREFIX TREE ######

class PrefixTree:

    #Use max_served_prefix to set a maximum served prefix size (e.g., /64 for IPv6)
    def __init__(self, prefix, max_served_prefix=None):
        self.prefix = prefix
        self.prefix_cls = type(prefix)
        if max_served_prefix is None:
            max_served_prefix = self.prefix_cls.MAX_PREFIX_SIZE
        self.max_served_prefix = max_served_prefix
        self.left = None
        self.right = None
        #When the full prefix is assigned
        self.full = False


    def find_prefix(self, prefix_size):
        ret, lret, rret = [None]*3
        if prefix_size > self.prefix.prefix_size and not self.full:
            if self.left is None:
                lret = self.prefix_cls(self.prefix.first_prefix_address(), self.prefix.prefix_size+1)
            else:
                lret = self.left.find_prefix(prefix_size)

            if self.right is None:
                rret = self.prefix_cls(self.prefix.last_prefix_address(), self.prefix.prefix_size+1)
            else:
                rret = self.right.find_prefix(prefix_size)

        #Now we look for the longer prefix to assign
        if not lret or (rret and rret.prefix_size > lret.prefix_size):
            ret = rret
        else:
            ret = lret
        return ret


    def assign_prefix(self, prefix):
        assert prefix in self.prefix
        if prefix.prefix_size > self.prefix.prefix_size:
            #Existing prefix on the left
            lp = self.prefix_cls(self.prefix.first_prefix_address(), self.prefix.prefix_size+1)
            #It's on the left branch
            if prefix in lp:
                if not self.left:
                    self.left = PrefixTree(lp)
                self.left.assign_prefix(prefix)
            #It's on the right branch
            else:
                rp = self.prefix_cls(self.prefix.last_prefix_address(), self.prefix.prefix_size+1)
                if not self.right:
                    self.right = PrefixTree(rp)
                self.right.assign_prefix(prefix)
        elif self.prefix == prefix:
            self.full = True
        else:
            raise RuntimeError("And you may ask yourself, well, how did I get here?")

    def get_prefix(self, prefix_size):
        ret = self.find_prefix(prefix_size)
        if not ret:
            raise NotEnoughAddresses
        #find_prefix returns the size of the largest available prefix in our space
        #not necessarily the one we asked for
        ret.prefix_size = prefix_size
        self.assign_prefix(ret)
        return ret

    def get_assigned_prefixes(self):
        ret = []
        if not self.right and not self.left and self.full:
            ret.append(self.prefix)
        else:
            if self.right:
                ret.extend(self.right.get_assigned_prefixes())
            if self.left:
                ret.extend(self.left.get_assigned_prefixes())
        return ret
