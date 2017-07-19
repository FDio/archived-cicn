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

from abc                        import ABCMeta

import sys

from netmodel.model.attribute   import Attribute, Multiplicity
from netmodel.model.key         import Key
from netmodel.model.type        import BaseType
from netmodel.model.mapper      import ObjectSpecification

# Warning and error messages

E_UNK_RES_NAME = 'Unknown resource name for attribute {} in {} ({}) : {}'

class ObjectMetaclass(type):
    """
    Object metaclass allowing non-uniform attribute declaration.
    """

    def __new__(mcls, name, bases, attrs):
        cls = super(ObjectMetaclass, mcls).__new__(mcls, name, bases, attrs)
        if (sys.version_info < (3, 6)):
            # Before Python 3.6, descriptor protocol does not include __set_name__.
            # We use a metaclass to emulate the functionality.
            for attr, obj in attrs.items():
                if isinstance(obj, ObjectSpecification):
                    obj.__set_name__(cls, attr)
        return cls

    def __init__(cls, class_name, parents, attrs):
        """
        Args:
            cls: The class type we're registering.
            class_name: A String containing the class_name.
            parents: The parent class types of 'cls'.
            attrs: The attribute (members) of 'cls'.
        """
        super().__init__(class_name, parents, attrs)
        cls._sanitize()

class Object(BaseType, metaclass = ObjectMetaclass):

    def __init__(self, **kwargs):
        """
        Object constructor.

        Args:
            kwargs: named arguments consisting in object attributes to be
                initialized at construction.
        """
        mandatory = { a.name for a in self.iter_attributes() if a.mandatory }

        for key, value in kwargs.items():
            attribute = self.get_attribute(key)
            if issubclass(attribute.type, Object):
                if attribute.is_collection:
                    new_value = list()
                    for x in value:
                        if isinstance(x, str):
                            resource = self._state.manager.by_name(x)
                        elif isinstance(x, UUID):
                            resource = self._state.manager.by_uuid(x)
                        else:
                            resource = x
                        if not resource:
                            raise LurchException(E_UNK_RES_NAME.format(key,
                                    self.name, self.__class__.__name__, x))
                        new_value.append(resource._state.uuid)
                    value = new_value
                else:
                    if isinstance(value, str):
                        resource = self._state.manager.by_name(value)
                    elif isinstance(value, UUID):
                        resource = self._state.manager.by_uuid(value)
                    else:
                        resource = value
                    if not resource:
                        raise LurchException(E_UNK_RES_NAME.format(key,
                                self.name, self.__class__.__name__, value))
                    value = resource._state.uuid
            setattr(self, key, value)
            mandatory -= { key }

        # Check that all mandatory atttributes have been set
        # Mandatory resource attributes will be marked as pending since they
        # might be discovered
        # Eventually, their absence will be discovered at runtime
        if mandatory:
            raise Exception('Mandatory attributes not set: %r' % (mandatory,))

        # Assign backreferences (we need attribute to be initialized, so it has
        # to be done at the end of __init__
        for other_instance, attribute in self.iter_backrefs():
            if attribute.is_collection:
                collection = getattr(other_instance, attribute.name)
                collection.append(self)
            else:
                setattr(other_instance, attribute.name, self)

    #--------------------------------------------------------------------------
    # Object model
    #--------------------------------------------------------------------------

    @classmethod
    def get_attribute(cls, key):
        return getattr(cls, key)

    @classmethod
    def _sanitize(cls):
        """
        This methods performs sanitization of the object declaration.

        More specifically:
         - it goes over all attributes and sets their name based on the python
           object attribute name.
         - it establishes mutual object relationships through reverse attributes.

        """
        cls._reverse_attributes = dict()
        cur_reverse_attributes = dict()
        for name, obj in vars(cls).items():
            if not isinstance(obj, Attribute):
                continue

            # Remember whether a reverse_name is defined before loading
            # inherited properties from parent
            has_reverse = bool(obj.reverse_name)

            # Handle overloaded attributes
            # By recursion, it is sufficient to look into the parent
            for base in cls.__bases__:
                if hasattr(base, name):
                    parent_attribute = getattr(base, name)
                    obj.merge(parent_attribute)
            assert obj.type, "No type for obj={} cls={}, base={}".format(obj, cls, base)

            # Handle reverse attribute
            #
            # NOTE: we need to do this after merging to be sure we get all
            #   properties inherited from parent (eg. multiplicity)
            #
            # See "Reverse attributes" section in BaseResource docstring.
            #
            # Continueing with the same example, let's detail how it is handled:
            #
            # Original declaration:
            # >>>
            # class Group(Resource):
            #     resources = Attribute(Resource, description = 'Resources belonging to the group',
            # 	    multiplicity = Multiplicity.ManyToMany,
            #             default = [],
            #             reverse_name = 'groups',
            #             reverse_description = 'Groups to which the resource belongs')
            # <<<
            #
            # Local variables:
            #   cls = <class 'vicn.resource.group.Group'>
            #   obj = <Attribute resources>
            #   obj.type = <class 'vicn.core.Resource'>
            #   reverse_attribute = <Attribute groups>
            #
            # Result:
            #    1) Group._reverse_attributes =
            #       { <Attribute resources> : [<Attribute groups>, ...], ...}
            #    2) Add attribute <Attribute groups> to class Resource
            #    3) Resource._reverse_attributes =
            #       { <Attribute groups> : [<Attribute resources], ...], ...}
            #
            if has_reverse:
                a = {
                    'name'                  : obj.reverse_name,
                    'description'           : obj.reverse_description,
                    'multiplicity'          : Multiplicity.reverse(obj.multiplicity),
                    'reverse_name'          : obj.name,
                    'reverse_description'   : obj.description,
                    'auto'                  : obj.reverse_auto,
                }

                # We need to use the same class as the Attribute !
                reverse_attribute = obj.__class__(cls,  **a)
                reverse_attribute.is_aggregate = True

                # 1) Store the reverse attributes to be later inserted in the
                # remote class, at the end of the function
                # TODO : clarify the reasons to perform this in two steps
                cur_reverse_attributes[obj.type] = reverse_attribute

                # 2)
                if not obj in cls._reverse_attributes:
                    cls._reverse_attributes[obj] = list()
                cls._reverse_attributes[obj].append(reverse_attribute)

                # 3)
                if not reverse_attribute in obj.type._reverse_attributes:
                    obj.type._reverse_attributes[reverse_attribute] = list()
                obj.type._reverse_attributes[reverse_attribute].append(obj)

        # Insert newly created reverse attributes in the remote class
        for kls, a in cur_reverse_attributes.items():
            setattr(kls, a.name, a)

    @classmethod
    def iter_attributes(cls, aggregates = False):
        for name in dir(cls):
            attribute = getattr(cls, name)
            if not isinstance(attribute, Attribute):
                continue
            if attribute.is_aggregate and not aggregates:
                continue

            yield attribute

    def get_attributes(self, aggregates = False):
        return list(self.iter_attributes(aggregates = aggregates))

    def get_attribute_names(self, aggregates = False):
        return set(a.name for a in self.iter_attributes(aggregates = \
                    aggregates))

    def get_attribute_dict(self, field_names = None, aggregates = False,
            uuid = True):
        assert not field_names or field_names.is_star()
        attributes = self.get_attributes(aggregates = aggregates)

        ret = dict()
        for a in attributes:
            if not a.is_set(self):
                continue
            value = getattr(self, a.name)
            if a.is_collection:
                ret[a.name] = list()
                for x in value:
                    if uuid and isinstance(x, Object):
                        x = x._state.uuid._uuid
                    ret[a.name].append(x)
            else:
                if uuid and isinstance(value, Object):
                    value = value._state.uuid._uuid
                ret[a.name] = value
        return ret

    @classmethod
    def iter_keys(cls):
        for name in dir(cls):
            key = getattr(cls, name)
            if not isinstance(key, Key):
                continue
            yield key

    @classmethod
    def get_keys(cls):
        return list(cls.iter_keys())

    def get_key_dicts(self):
        return [{attribute: self.get(attribute.name) for attribute in key} for key in self.iter_keys()]

    def get_tuple(self):
        return (self.__class__, self._get_attribute_dict())

    def format(self, fmt):
        return fmt.format(**self.get_attribute_dict(uuid = False))

    def iter_backrefs(self):
        for base in self.__class__.mro():
            if not hasattr(base, '_reverse_attributes'):
                continue
            for attr, rattrs in base._reverse_attributes.items():
                instances = getattr(self, attr.name)
                if not attr.is_collection:
                    instances = [instances]
                for instance in instances:
                    #  - instance = node
                    if instance in (None, NEVER_SET):
                        continue
                    for rattr in rattrs:
                        yield instance, rattr

    #--------------------------------------------------------------------------
    # Accessors
    #--------------------------------------------------------------------------

    @classmethod
    def has_attribute(cls, name):
        return name in [a.name for a in cls.attributes()]

    def get(self, attribute_name):
        raise NotImplementedError

    def set(self, attribute_name, value):
        raise NotImplementedError
