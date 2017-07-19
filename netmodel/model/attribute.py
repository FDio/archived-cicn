#!/usr/bin/env python2
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

from netmodel.model.mapper          import ObjectSpecification
from netmodel.model.type            import Type, Self
from netmodel.util.misc             import is_iterable
from netmodel.model.collection      import Collection

log = logging.getLogger(__name__)
instance_dict = operator.attrgetter('__dict__')

class NEVER_SET: None

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
        'description',
        'default',
        'mandatory',
        'multiplicity',
        'ro',
        'auto',
        'func',
        'reverse_name',
        'reverse_description',
        'reverse_auto'
    ]

    def __init__(self, *args, **kwargs):
        for key in kwargs.keys():
            if not key in self.properties:
                raise ValueError("Invalid attribute property {}".format(key))
        for key in self.properties:
            value = kwargs.pop(key, NEVER_SET)
            setattr(self, key, value)

        if len(args) == 1:
            self.type, = args
            # self.type is optional since the type can be inherited. Although we
            # will have to verify the attribute is complete at some point
            if isinstance(self.type, str):
                self.type = Type.from_string(self.type)
            assert self.type is Self or Type.exists(self.type)

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
                value = Collection(default)
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
            if not isinstance(value, Collection):
                value = Collection(value)
                value._attribute = self
                value._instance  = instance

        instance_dict(instance)[self.name] = value
        if hasattr(instance, '_state'):
            instance._state.attr_dirty.add(self.name)
            instance._state.dirty = True

    def __delete__(self, instance):
        raise NotImplementedError

    def __set_name__(self, owner, name):
        self.name = name
        self.owner = owner

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
