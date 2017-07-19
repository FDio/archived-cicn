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

from netmodel.model.type        import Inet4Prefix, Inet6Prefix

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


    def find_prefix(self, prefix_len):
        ret, lret, rret = [None]*3
        if prefix_len > self.prefix.prefix_len and not self.full:
            if self.left is None:
                lret = self.prefix_cls(self.prefix.first_prefix_address(), self.prefix.prefix_len+1)
            else:
                lret = self.left.find_prefix(prefix_len)

            if self.right is None:
                rret = self.prefix_cls(self.prefix.last_prefix_address(), self.prefix.prefix_len+1)
            else:
                rret = self.right.find_prefix(prefix_len)

        #Now we look for the longer prefix to assign
        if not lret or (rret and rret.prefix_len > lret.prefix_len):
            ret = rret
        else:
            ret = lret
        return ret


    def assign_prefix(self, prefix):
        assert prefix in self.prefix
        if prefix.prefix_len > self.prefix.prefix_len:
            #Existing prefix on the left
            lp = self.prefix_cls(self.prefix.first_prefix_address(), self.prefix.prefix_len+1)
            #It's on the left branch
            if prefix in lp:
                if not self.left:
                    self.left = PrefixTree(lp)
                self.left.assign_prefix(prefix)
            #It's on the right branch
            else:
                rp = self.prefix_cls(self.prefix.last_prefix_address(), self.prefix.prefix_len+1)
                if not self.right:
                    self.right = PrefixTree(rp)
                self.right.assign_prefix(prefix)
        elif self.prefix == prefix:
            self.full = True
        else:
            raise RuntimeError("And you may ask yourself, well, how did I get here?")

    def get_prefix(self, prefix_len):
        ret = self.find_prefix(prefix_len)
        if not ret:
            raise NotEnoughAddresses
        #find_prefix returns the size of the largest available prefix in our space
        #not necessarily the one we asked for
        ret.ip_address = ret.first_prefix_address()
        ret.prefix_len = prefix_len
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
