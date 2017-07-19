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

class FIBEntry:
    def __init__(self, prefix, next_hops = None):
        if next_hops is None:
            next_hops = set()

        self._prefix = prefix
        self._next_hops = next_hops

    def update(self, next_hops = None):
        if not next_hops:
            return
        self._next_hops |= next_hops

    def remove(self, next_hops = None):
        if not next_hops:
            return
        self._next_hops &= next_hops

class FIB:
    def __init__(self):
        self._entries = dict()

    def add(self, prefix, next_hops = None):
        if prefix not in self._entries:
            self._entries[prefix] = FIBEntry(prefix, next_hops)
        else:
            self._entries[prefix].update(next_hops)

    def update(self, prefix, next_hops = None):
        entry = self._entries.get(prefix)
        if not entry:
            raise Exception('prefix not found')
        entry.update(next_hops)

    def remove(self, prefix, next_hops = None):
        if next_hop:
            entry = self._entries.get(prefix)
            if not entry:
                raise Exception('prefix not found')
            entry.remove(next_hops)
            return

        del self._entries[prefix]

    def get(self, object_name):
        for entry in self._entries.values():
            if entry._prefix.object_name == object_name:
                return entry._next_hops
        return None
