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

from netmodel.model.mapper      import ObjectSpecification
from netmodel.model.type        import is_type
from netmodel.util.meta         import inheritors
from netmodel.util.misc         import is_iterable
from vicn.core.sa_collections   import InstrumentedList, _list_decorators

log = logging.getLogger(__name__)
instance_dict = operator.attrgetter('__dict__')

class NEVER_SET: None

#------------------------------------------------------------------------------
# Attribute Multiplicity
#------------------------------------------------------------------------------

class Multiplicity:
    _1_1 = '1_1'
    _1_N = '1_N'
    _N_1 = 'N_1'
    _N_N = 'N_N'
    

    @staticmethod
    def reverse(value):
        reverse_map = {
            Multiplicity._1_1: Multiplicity._1_1,
            Multiplicity._1_N: Multiplicity._N_1,
            Multiplicity._N_1: Multiplicity._1_N,
            Multiplicity._N_N: Multiplicity._N_N,
        }
        return reverse_map[value]


# Default attribute properties values (default to None)
DEFAULT = {
    'multiplicity'  :  Multiplicity._1_1,
    'mandatory'     : False,
}

#------------------------------------------------------------------------------
# Attribute
#------------------------------------------------------------------------------

class Attribute(abc.ABC, ObjectSpecification):
    properties = [
        'name',
        'type',
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
            assert is_type(self.type)

        self.is_aggregate = False

        self._reverse_attributes = list()
        
    #--------------------------------------------------------------------------
    # Display
    #--------------------------------------------------------------------------

    def __repr__(self):
        return '<Attribute {}>'.format(self.name)

    __str__ = __repr__

    #--------------------------------------------------------------------------
    # Descriptor protocol
    #
    # see. https://docs.python.org/3/howto/descriptor.html
    #--------------------------------------------------------------------------

    def __get__(self, instance, owner=None):
        if instance is None:
            return self

        value = instance_dict(instance).get(self.name, NEVER_SET)
        
        # Case : collection attribute
        if self.is_collection:
            if value is NEVER_SET:
                if isinstance(self.default, types.FunctionType):
                    default = self.default(instance)
                else:
                    default = self.default
                value = InstrumentedList(default)
                value._attribute = self
                value._instance  = instance
                self.__set__(instance, value)
                return value
            return value 

        # Case : scalar attribute

        if value in (None, NEVER_SET) and self.auto not in (None, NEVER_SET):
            # Automatic instanciation
            if not self.requirements in (None, NEVER_SET) and \
                    self.requirements:
                log.warning('Ignored requirement {}'.format(self.requirements))
            value = instance.auto_instanciate(self)
            self.__set__(instance, value)
            return value

        if value is NEVER_SET:
            if isinstance(self.default, types.FunctionType):
                value = self.default(instance)
            else:
                value = copy.deepcopy(self.default)
            self.__set__(instance, value)
            return value

        return value

    def __set__(self, instance, value):
        if instance is None:
            return

        if self.is_collection:
            if not isinstance(value, InstrumentedList):
                value = InstrumentedList(value) 
                value._attribute = self
                value._instance  = instance

        instance_dict(instance)[self.name] = value
        if hasattr(instance, '_state'):
            instance._state.attr_dirty.add(self.name)
            instance._state.dirty = True

    def __delete__(self, instance):
        raise NotImplementedError

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

    # Shortcuts

    def has_reverse_attribute(self):
        return self.reverse_name and self.multiplicity

    @property
    def is_collection(self):
        return self.multiplicity in (Multiplicity._1_N, Multiplicity._N_N)

    def is_set(self, instance):
        return self.name in instance_dict(instance)

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

    #--------------------------------------------------------------------------
    # Attribute values
    #--------------------------------------------------------------------------

    def _handle_getitem(self, instance, item):
        return item

    def _handle_add(self, instance, item):
        instance._state.dirty = True
        instance._state.attr_dirty.add(self.name)
        print('marking', self.name, 'as dirty')
        return item

    def _handle_remove(self, instance, item):
        instance._state.dirty = True
        instance._state.attr_dirty.add(self.name)
        print('marking', self.name, 'as dirty')

    def _handle_before_remove(self, instance):
        pass

    #--------------------------------------------------------------------------
    # Attribute values
    #--------------------------------------------------------------------------

class Relation(Attribute):
    properties = Attribute.properties[:]
    properties.extend([
        'reverse_name',
        'reverse_description',
        'multiplicity',
    ])

class SelfRelation(Relation):
    def __init__(self, *args, **kwargs):
        if args:
            if not len(args) == 1:
                raise ValueError('Bad initialized for SelfRelation')
            name, = args
            super().__init__(name, None, *args, **kwargs)
        else:
            super().__init__(None, *args, **kwargs)
