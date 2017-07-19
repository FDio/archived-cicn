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

from netmodel.model.uuid            import UUID
from vicn.core.attribute            import NEVER_SET

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

class PendingValue:
    def __init__(self, value = None):
        self.clear(value)

    def clear(self, value=NEVER_SET):
        self.value = NEVER_SET
        self.operations = list()

    def trigger(self, action, value, cur_value = None):

        if self.value is NEVER_SET:
            #XXX Shouldn't we set it to None if it is demanded?
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
