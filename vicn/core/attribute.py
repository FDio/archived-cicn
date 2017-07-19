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

from netmodel.model.attribute       import Attribute as BaseAttribute, NEVER_SET
from netmodel.model.attribute       import Multiplicity, DEFAULT
from netmodel.model.type            import Self
from netmodel.model.uuid            import UUID
from netmodel.util.misc             import is_iterable
from vicn.core.requirement          import Requirement, RequirementList
from vicn.core.state                import Operations

log = logging.getLogger(__name__)

#------------------------------------------------------------------------------
# Attribute
#------------------------------------------------------------------------------

class Attribute(BaseAttribute):
    properties = BaseAttribute.properties
    properties.extend([
        'requirements',
        'remote_default'
    ])

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # Post processing attribute properties
        if self.requirements is not NEVER_SET:
            self.requirements = RequirementList(self.requirements)


    #--------------------------------------------------------------------------
    # Descriptor protocol
    #
    # see. https://docs.python.org/3/howto/descriptor.html
    #--------------------------------------------------------------------------

    # XXX Overloaded & simpler

    def __get__(self, instance, owner=None):
        if not instance:
            return self

        return instance.get(self.name, blocking=False)

    def __set__(self, instance, value):
        if not instance:
            raise NotImplementedError('Setting default value not implemented')

        instance.set(self.name, value, blocking=False)

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
            raise InstrumentedListException

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
            raise InstrumentedListException

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
            raise InstrumentedListException

    def handle_getitem(self, instance, item):
        if isinstance(item, UUID):
            from vicn.core.resource_mgr import ResourceManager
            return ResourceManager().by_uuid(item)
        return item

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

# XXX Move this to object, be careful of access to self._reference !
class Reference:
    """
    Value reference.

    Attribute value refers to attribute value on a different resource.
    Use resource = Self to point to another attribute of the same resource.
    """

    def __init__(self, resource, attribute=None):
        self._resource = resource
        self._attribute = attribute

    def get_proxy(self):
        if self._resource is Self:
            resource = getattr(self, self._attribute)
        else:
            resource = getattr(self._resource, self._attribute)
        return resource

    def get(self, attribute_name):
        return self.get_proxy().get(attribute_name)

    def __iter__(self):
        return iter(self.get_proxy())
