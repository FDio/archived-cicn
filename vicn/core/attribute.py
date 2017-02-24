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

import abc
import copy
import logging
import operator
import types

from netmodel.model.mapper         import ObjectSpecification
from netmodel.model.type           import Type, Self
from netmodel.util.meta            import inheritors
from netmodel.util.misc            import is_iterable
from vicn.core.exception           import VICNListException
from vicn.core.requirement         import Requirement, RequirementList
from vicn.core.sa_collections      import InstrumentedList
from vicn.core.state               import UUID, NEVER_SET, Operations

log = logging.getLogger(__name__)

#------------------------------------------------------------------------------
# Attribute Multiplicity
#------------------------------------------------------------------------------

class Multiplicity:
    OneToOne = '1_1'
    OneToMany = '1_N'
    ManyToOne = 'N_1'
    ManyToMany = 'N_N'
    

    @staticmethod
    def reverse(value):
        reverse_map = {
            Multiplicity.OneToOne: Multiplicity.OneToOne,
            Multiplicity.OneToMany: Multiplicity.ManyToOne,
            Multiplicity.ManyToOne: Multiplicity.OneToMany,
            Multiplicity.ManyToMany: Multiplicity.ManyToMany,
        }
        return reverse_map[value]


# Default attribute properties values (default to None)
DEFAULT = {
    'multiplicity'  :  Multiplicity.OneToOne,
    'mandatory'     : False,
}

#------------------------------------------------------------------------------
# Attribute
#------------------------------------------------------------------------------

class Attribute(abc.ABC, ObjectSpecification):
    properties = [
        'name',
        'type',
        'key',
        'description',
        'default',
        'choices',
        'mandatory',
        'multiplicity',
        'ro',
        'auto',
        'func',
        'requirements',
        'reverse_name',
        'reverse_description',
        'reverse_auto'
    ]

    def __init__(self, *args, **kwargs):
        for key in Attribute.properties:
            value = kwargs.pop(key, NEVER_SET)
            setattr(self, key, value)

        if len(args) == 1:
            self.type, = args
        elif len(args) == 2:
            self.name, self.type = args

        # self.type is optional since the type can be inherited. Although we
        # will have to verify the attribute is complete at some point
        if self.type:
            if isinstance(self.type, str):
                self.type = Type.from_string(self.type)
            assert self.type is Self or Type.exists(self.type)

        # Post processing attribute properties
        if self.requirements is not NEVER_SET:
            self.requirements = RequirementList(self.requirements)

        self.is_aggregate = False

        self._reverse_attributes = list()
        
    #--------------------------------------------------------------------------
    # Display
    #--------------------------------------------------------------------------

    def __repr__(self):
        return '<Attribute {}>'.format(self.name)

    __str__ = __repr__

    # The following functions are required to allow comparing attributes, and
    # using them as dict keys

    def __eq__(self, other):
        return self.name == other.name

    def __hash__(self):
        return hash(self.name)

    #--------------------------------------------------------------------------
    # Descriptor protocol
    #
    # see. https://docs.python.org/3/howto/descriptor.html
    #--------------------------------------------------------------------------

    def __get__(self, instance, owner=None):
        if not instance:
            return self

        return instance.get(self.name, blocking=False)

    def __set__(self, instance, value):
        if not instance:
            raise NotImplementedError('Setting default value not implemented')

        instance.set(self.name, value, blocking=False)

    def __delete__(self, instance):
        raise NotImplementedError

    #--------------------------------------------------------------------------

    def do_list_add(self, instance, value):
        if instance.is_local_attribute(self.name):
            from vicn.core.resource import Resource
            if isinstance(value, Resource):
                value = value.get_uuid()
            return value
        else:
            try: 
                cur_value = vars(instance)[self.name]
                if self.is_collection:
                    # copy the list
                    cur_value = list(cur_value)
            except KeyError as e:
                cur_value = None
                if self.is_collection:
                    cur_value = list()

            instance._state.dirty[self.name].trigger(Operations.LIST_ADD, 
                    value, cur_value)

            # prevent instrumented list to perform operation
            raise VICNListException 

    def do_list_remove(self, instance, value):
        if instance.is_local_attribute(self.name):
            from vicn.core.resource import Resource
            if isinstance(value, Resource):
                value = value.get_uuid()
            return value
        else:
            cur_value = vars(instance)[self.name]
            if self.is_collection:
                # copy the list
                cur_value = list(cur_value)
            instance._state.dirty[self.name].trigger(Operations.LIST_REMOVE, 
                    value, cur_value)

            # prevent instrumented list to perform operation
            raise VICNListException 

    def do_list_clear(self, instance):
        if instance.is_local_attribute(self.name):
            return
        else:
            cur_value = vars(instance)[self.name]
            if self.is_collection:
                # copy the list
                cur_value = list(cur_value)
            instance._state.dirty[self.name].trigger(Operations.LIST_CLEAR, 
                    value, cur_value)

            # prevent instrumented list to perform operation
            raise VICNListException 

    def handle_getitem(self, instance, item):
        if isinstance(item, UUID):
            from vicn.core.resource_mgr import ResourceManager
            return ResourceManager().by_uuid(item)
        return item

    #--------------------------------------------------------------------------
    # Accessors
    #--------------------------------------------------------------------------

    def __getattribute__(self, name):
        value = super().__getattribute__(name)
        if value is NEVER_SET:
            if name == 'default':
                return list() if self.is_collection else None
            return DEFAULT.get(name, None)
        return value

    def has_reverse_attribute(self):
        return self.reverse_name and self.multiplicity

    @property
    def is_collection(self):
        return self.multiplicity in (Multiplicity.OneToMany, 
                Multiplicity.ManyToMany)

    def is_set(self, instance):
        return instance.is_set(self.name)

    #--------------------------------------------------------------------------
    # Operations
    #--------------------------------------------------------------------------

    def merge(self, parent):
        for prop in Attribute.properties:
            # NOTE: we cannot use getattr otherwise we get the default value,
            # and we never override
            value = vars(self).get(prop, NEVER_SET)
            if value is not NEVER_SET and not is_iterable(value):
                continue

            parent_value = vars(parent).get(prop, NEVER_SET)
            if parent_value is NEVER_SET:
                continue

            if parent_value:
                if is_iterable(value):
                    value.extend(parent_value)
                else:
                    setattr(self, prop, parent_value)

#------------------------------------------------------------------------------

class Reference:
    """
    Value reference.

    Attribute value refers to attribute value on a different resource.
    Use resource = Self to point to another attribute of the same resource.
    """

    def __init__(self, resource, attribute=None):
        self._resource = resource
        self._attribute = attribute
