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

import asyncio
import random
import string

class NEVER_SET:
    pass

# Separator for components of the UUID
UUID_SEP = '-'

# Length of the random component of the UUID
UUID_LEN = 5

class ResourceState:
    UNINITIALIZED       = 'UNINITIALIZED'
    PENDING_DEPS        = 'PENDING_DEPS'
    DEPS_OK             = 'DEPS_OK'
    PENDING_INIT        = 'PENDING_INIT'
    INITIALIZED         = 'INITIALIZED'
    PENDING_GET         = 'PENDING_GET'
    GET_DONE            = 'GET_DONE'
    PENDING_KEYS        = 'PENDING_KEYS'
    KEYS_OK             = 'KEYS_OK'
    PENDING_CREATE      = 'PENDING_CREATE'
    CREATED             = 'CREATED'
    DIRTY               = 'DIRTY'
    CLEAN               = 'CLEAN'
    PENDING_UPDATE      = 'PENDING_UPDATE'
    PENDING_DELETE      = 'PENDING_DELETE'
    DELETED             = 'DELETED'
    ERROR               = 'ERROR'

class AttributeState:
    UNINITIALIZED       = 'UNINITIALIZED'
    INITIALIZED         = 'INITIALIZED'
    DIRTY               = 'DIRTY'
    PENDING_INIT        = 'PENDING_INIT'
    PENDING_UPDATE      = 'PENDING_UPDATE'
    CLEAN               = 'CLEAN'
    ERROR               = 'ERROR'
    RESET               = 'RESET'

class Operations:
    SET = 'set'
    LIST_ADD = 'add'
    LIST_REMOVE = 'remove'
    LIST_CLEAR = 'clear'

class UUID:
    def __init__(self, name, cls):
        self._uuid = self._make_uuid(name, cls)

    def _make_uuid(self, name, cls):
        """Generate a unique resource identifier

        The UUID consists in the type of the resource, to which is added a
        random identifier of length UUID_LEN. Components of the UUID are
        separated by UUID_SEP.
        """
        uuid = ''.join(random.choice(string.ascii_uppercase + string.digits) 
                for _ in range(UUID_LEN))
        if name:
            uuid = name # + UUID_SEP + uuid
        return UUID_SEP.join([cls.__name__, uuid])

    def __repr__(self):
        return '<UUID {}>'.format(self._uuid)

    def __lt__(self, other):
        return self._uuid < other._uuid

    __str__ = __repr__

class PendingValue:
    def __init__(self, value = None):
        self.clear(value)

    def clear(self, value=NEVER_SET):
        self.value = NEVER_SET
        self.operations = list()

    def trigger(self, action, value, cur_value = None):

        if self.value is NEVER_SET:
            if cur_value is not None:
                self.value = cur_value

        if action == Operations.SET:
            self.value = value
            self.operations = [(Operations.SET, value)] 
        elif action == Operations.LIST_CLEAR:
            self.value = list()
            self.operations = [(Operations.LIST_CLEAR, None)]
        else:
            if action == Operations.LIST_ADD:
                self.value.append(value)
            elif action == Operations.LIST_REMOVE:
                self.value.remove(value)
            else:
                raise RuntimeError
            self.operations.append((action, value))

class InstanceState:
    def __init__(self, manager, instance, name = None):

        # Unique identifier for the instance. This is useful for relation
        # between resources
        self.uuid = UUID(name, instance.__class__)
        self.instance       = instance

        # Instance manager
        self.manager        = manager

        # Events
        self.events         = dict()

        # Stores the requested value : attribute_name -> requested operations =
        # LIST set add remove clear
        self.dirty          = dict()


        # Initialize resource state
        self.lock = asyncio.Lock() 
        self.write_lock = asyncio.Lock()
        self.state          = ResourceState.UNINITIALIZED
        self.clean = asyncio.Event()
        self.clean.clear()
        self.init  = asyncio.Event()
        self.init.clear()
        self.change_event = asyncio.Event()
        self.change_event.clear()
        self.change_success = None
        self.change_value = None
        self.log = list()

        self.attr_lock = dict()
        self.attr_init = dict()
        self.attr_clean = dict()
        self.attr_state = dict()
        self.attr_change_event = dict()
        self.attr_change_success = dict()
        self.attr_change_value= dict()
        self.attr_log = dict()
        # Initialize attribute state
        for attribute in instance.iter_attributes():
            self.attr_lock[attribute.name] = asyncio.Lock() 
            self.attr_init[attribute.name] = asyncio.Event()
            self.attr_clean[attribute.name] = asyncio.Event()
            self.attr_state[attribute.name] = AttributeState.UNINITIALIZED
            self.attr_change_event[attribute.name] = asyncio.Event()
            self.attr_change_event[attribute.name].clear()
            self.attr_change_success[attribute.name] = None
            self.attr_change_value[attribute.name] = None
            self.dirty[attribute.name] = PendingValue(NEVER_SET)
            self.attr_log[attribute.name] = list()

    def set_dirty(self, attr_name):
        self.attr_dirty.add(attr_name)
        self.manager.set_dirty(self.uuid)

    def trigger(self, attribute_name, action, value):
        self.dirty[attribute_name].trigger(action, value)
