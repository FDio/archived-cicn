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

import enum

from netmodel.model.mapper      import ObjectSpecification
from vicn.core.exception        import VICNException

#------------------------------------------------------------------------------
# Enums
#------------------------------------------------------------------------------

class RequirementScope(enum.Enum):
    INSTANCE    = 'Instance'
    CLASS       = 'Class'

#------------------------------------------------------------------------------
# Exceptions
#------------------------------------------------------------------------------

class RequirementError(VICNException):

    def __init__(self, instance, attr):
        super().__init__()
        self._instance = instance
        self._attr = attr

    def __str__(self):
        return "Requirement on {}.{} could not be satisfied:".format(
                self._instance, self._attr)

class RequiredAttributeError(RequirementError):

    def __str__(self):
        return super().__str__() + "could not find attribute {}".format(
                self._attr)

class RequiredPropertyError(RequirementError):
    def __init__(self, instance, attr, prop):
        super().__init__(instance, attr)
        self._prop = prop

    def __str__(self):
        return super().__str__()+ "property {} is not verified".format(
                self._prop)

#------------------------------------------------------------------------------
# Class: Property
#------------------------------------------------------------------------------

class Property:

    TYPE_ANY_OF = 0
    #XXX cant think of a good use case for that
    #TYPE_ALL_OF = 1

    def __init__(self, value, property_type=TYPE_ANY_OF):
        self._type = property_type
        try:
            self._value = set(value)
        except TypeError: #value is not iterable, it is a single value
            self._value = set()
            self._value.add(value)


    @property
    def property_type(self):
        return self._type

    @property
    def value(self):
        return self._value

    def check(self, value):
        return value in self._value

    def __str__(self):
        return str(self._value)

    def merge(self, other):
        assert self._type is other.property_type, \
            "Properties must be of same type to be merged"

        #if self._type is TYPE_ANY_OF:
        self._value.intersection_update(other.value)
        #elif self._type is TYPE_ALL_OF:
        #    self._value.union_update(other.value)

#------------------------------------------------------------------------------
# Class: Requirement
#------------------------------------------------------------------------------
                
class Requirement(ObjectSpecification):
    """Resource requirement

    This class allows to specify a requirement on a given resource, or on a
    class of resources (all instances of a given class).
    """

    #--------------------------------------------------------------------------
    # Constructor
    #--------------------------------------------------------------------------

    def __init__(self, requirement_type, properties = None, 
            capabilities = None, scope = RequirementScope.INSTANCE, 
            fatal = True, must_be_setup = False):
        """
        Args:
            requirement_type (): the attribute on which the requirement is made
            properties (Optional[XXX]): XXX (defaults to None)
            scope (Optional[enum RequirementScope]): Is the requirement dealing
                with an instance, or a class (all instance of the class)
                (defaults to RequirementScope.INSTANCE) 
            fatal (Optional[bool]): is the failure of the requirement fatal
                (raises an error), or not (raises a warning) (defaults to True)
            must_be_setup (Optional[bool]): defaults to False
        """
        self._type = requirement_type
        self._properties = {}
        if properties:
            for prop in properties:
                self._properties[prop] = Property(properties[prop])
        self._capabilities = capabilities if capabilities else set()
        self._scope = scope
        self._fatal = fatal
        self._must_be_up = must_be_setup

    #--------------------------------------------------------------------------
    # Accessors and properties
    #--------------------------------------------------------------------------

    @property
    def properties(self):
        return self._properties

    @property
    def requirement_type(self):
        return self._type

    @property
    def must_be_up(self):
        return self._must_be_up

    #--------------------------------------------------------------------------
    # Display
    #--------------------------------------------------------------------------

    def __str__(self):
        prop_str = "{" + ",".join(map(lambda x: "'{}': {}".format(x, 
                        self._properties[x]), self._properties.keys())) +"}"
        return "<type={}, properties={}, must_be_up={}>".format(self._type, 
                prop_str, self._must_be_up)

    #--------------------------------------------------------------------------
    # Requirement operators
    #--------------------------------------------------------------------------

    def check(self, instance):
        if not hasattr(instance, self._type):
            raise RequiredAttributeError(instance, self._type)

        instance_attr = getattr(instance, self._type)
        if not instance_attr:
            raise TypeError("instance_attr is none")

        for prop in self.properties:
            if not hasattr(instance_attr, prop):
                raise RequiredAttributeError(instance, self._type)
            if not self._properties[prop].check(getattr(instance_attr, prop)):
                raise RequiredPropertyError(instance, self._type, prop)

        return True

    #--------------------------------------------------------------------------
    # Requirement logic
    #--------------------------------------------------------------------------

    def merge(self, other):
        assert other.requirement_type == self._type, \
                       "Cannot merge Requirements with different types"

        for prop in other.properties:
            if prop in self._properties:
                self._properties[prop].merge(other.properties[prop])
            else:
                self._properties[prop] = other.properties[prop]

        if other._capabilities:
            self._capabilities |= other._capabilities

#------------------------------------------------------------------------------
# Class: Requirement list
#------------------------------------------------------------------------------

class RequirementList(list):

    def __init__(self,x=None):
        super().__init__()
        if x:
            self.extend(x)

    def append(self,x):
        assert isinstance(x,Requirement)
        # XXX O(n) right now, might be able to do better
        for req in self:
            if req.requirement_type == x.requirement_type:
                req.merge(x)
                return

        super().append(x)

    def extend(self, x):
        for y in x:
            self.append(y)
