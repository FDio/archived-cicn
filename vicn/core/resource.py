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
import copy
import logging
import operator
import random
import string
import sys
import traceback
import types

from abc                            import ABC, ABCMeta
from threading                      import Event as ThreadEvent

# LXD workaround
from pylxd.exceptions import NotFound as LXDAPIException

from netmodel.model.collection      import Collection
from netmodel.model.key             import Key
from netmodel.model.mapper          import ObjectSpecification
from netmodel.model.object          import Object
from netmodel.model.type            import String, Bool, Integer, Dict
from netmodel.model.type            import BaseType, Self
from netmodel.model.uuid            import UUID
from netmodel.util.deprecated       import deprecated
from netmodel.util.singleton        import Singleton
from vicn.core.attribute            import Attribute, Multiplicity, Reference
from vicn.core.attribute            import NEVER_SET
from vicn.core.commands             import ReturnValue
from vicn.core.event                import Event, AttributeChangedEvent
from vicn.core.exception            import VICNException, ResourceNotFound
from vicn.core.exception            import VICNWouldBlock
from vicn.core.resource_factory     import ResourceFactory
from vicn.core.requirement          import Requirement, Property
from vicn.core.scheduling_algebra   import SchedulingAlgebra
from vicn.core.state                import ResourceState
from vicn.core.state                import Operations, InstanceState
from vicn.core.task                 import run_task, BashTask

log = logging.getLogger(__name__)

NAME_SEP = '-'

# Warning and error messages

W_UNK_ATTR = 'Ignored unknown attribute {} for resource {}'
E_UNK_RES_NAME = 'Unknown resource name for attribute {} in {} ({}) : {}'
E_GET_NON_LOCAL = 'Cannot get non-local attribute {} for resource {}'
E_AUTO_UNM = 'Trying to auto-instanciate attribute {} on unmanaged resource {}'

#------------------------------------------------------------------------------
# Resource category
#------------------------------------------------------------------------------

# A base resource is not instanciated itself but uses delegates. Which one to
# use is resolved during initialization
class TopLevelResource: pass

class FactoryResource(TopLevelResource): pass
class CategoryResource(TopLevelResource): pass

##------------------------------------------------------------------------------
#
#class ResourceMetaclass(ABCMeta, ObjectSpecification):
#    def __init__(cls, class_name, parents, attrs):
#        """
#        Args:
#            cls: The class type we're registering.
#            class_name: A String containing the class_name.
#            parents: The parent class types of 'cls'.
#            attrs: The attribute (members) of 'cls'.
#        """
#        super().__init__(class_name, parents, attrs)
#
#        # We use the metaclass to create attributes for instance, even before
#        # the Resource Factory is called. They are needed both for initializing
#        # attributes and reverse attributes, in whatever order. Only class
#        # creation allow us to clear _attributes, otherwise, we will just add
#        # those from the parent, siblings, etc...
#        cls._sanitize()

#------------------------------------------------------------------------------

class BaseResource(Object):  #, ABC, metaclass=ResourceMetaclass):
    """Base Resource class

    The base Resource class implements all the logic related to resource
    instances.

    See also :
     * ResourceManager : logic related to class instanciation
     * Resource metaclass : logic related to class construction
     * ResourceFactory : logic related to available classes and mapping from
        name to type

    Internal attributes:

     -  _reverse_attributes: a dict mapping attribute objects with the class
        that declared the reverse attribute.

        For instance, a Group declares a collection of Resource objects through
        its resources attributes. It also mentions a reverse attribute named
        'groups'. This means every Resource class will be equipped with a
        groups attribute, being a collection of Group objects.

        Resource._reverse_attributes = { <Attribute: groups> : Resource }
    """

    __type__ = TopLevelResource

    name = Attribute(String, description = 'Alias name for the resource')
    managed = Attribute(Bool, description = 'Flag: resource is managed',
            default = True)
    owner = Attribute(Self, description = 'Owning resource', default = None)
    data = Attribute(Dict, description = 'User data')

    #---------------------------------------------------------------------------
    # Constructor
    #---------------------------------------------------------------------------

    def __new__(cls, *args, **kwargs):
        """
        We implement a "factory method" design pattern in the constructor...
        """
        # Ensure the resource factory exists and has been initialized, and thus
        # that Resource objects are fully created
        from vicn.core.resource_mgr import ResourceManager

        ResourceFactory()

        delegate = ResourceManager().get_resource_with_capabilities(cls, set())
        if not delegate:
            log.error('No delegate for abstract resource : %s', cls.__name__)
            raise VICNException

        instance = super().__new__(delegate)

        return instance

    def __init__(self, *args, **kwargs):
        from vicn.core.resource_mgr import ResourceManager

        # Cache dependencies
        self._deps = None

        # Internal data tag for resources
        self._internal_data = dict()

        mandatory = { a.name for a in self.iter_attributes() if a.mandatory }

        for key, value in kwargs.items():
            attribute = self.get_attribute(key)
            if attribute is None:
                log.warning(W_UNK_ATTR.format(key, self.get_type()))
                continue

            if value and issubclass(attribute.type, Resource):
                if attribute.is_collection:
                    new_value = list()
                    for x in value:
                        if isinstance(x, str):
                            resource = ResourceManager().by_name(x)
                        elif isinstance(x, UUID):
                            resource = ResourceManager().by_uuid(x)
                        else:
                            resource = x
                        if not resource:
                            raise VICNException(E_UNK_RES_NAME.format(key,
                                        self.name, self.__class__.__name__, x))
                        element = resource if isinstance(resource, Reference) \
                                else resource._state.uuid
                        new_value.append(element)
                    value = new_value
                else:
                    if isinstance(value, str):
                        resource = ResourceManager().by_name(value)
                    elif isinstance(value, UUID):
                        resource = ResourceManager().by_uuid(value)
                    else:
                        resource = value
                    if not resource:
                        raise VICNException(E_UNK_RES_NAME.format(key,
                                    self.name, self.__class__.__name__, value))
                    value = value if isinstance(resource, Reference) \
                            else resource._state.uuid
            self.set(key, value, blocking=False)
            mandatory -= { key }

        # Check that all mandatory atttributes have been set
        # Mandatory resource attributes will be marked as pending since they
        # might be discovered
        # Eventually, their absence will be discovered at runtime
        if mandatory:
            raise VICNException('Mandatory attributes not set: %r' % (mandatory,))

        # Check requirements and default values
        for attr in self.iter_attributes():
            # XXX fix for lambda attributes, since initialization makes no sense
            if hasattr(attr, 'func') and attr.func:
                continue
            if attr.name not in kwargs:
                default = self.get_default_collection(attr) if attr.is_collection else \
                        self.get_default(attr)
                if vars(attr)['default'] != NEVER_SET:
                    self.set(attr.name, default, blocking=False)
            if issubclass(attr.type, Resource) and attr.requirements:
                for req in attr.requirements:
                    instance = self.get(attr.name)
                    if instance is None:
                        continue
                    ResourceManager().add_instance_requirement(instance, req)

        self._subresources = None

    def __after__(self):
        return tuple()

    def __after_init__(self):
        return tuple()

    def __subresources__(self):
        return None

    def set_subresources(self, subresources):
        if not subresources:
            return

        # Add state to operators
        for sr in subresources:
            if not hasattr(sr, '_state'):
                sr._state = InstanceState(self._state.manager, sr)

        self._subresources = subresources

    def get_uuid(self):
        return self._state.uuid

    def from_uuid(self, uuid):
        return self._state.manager.by_uuid(uuid)

    #--------------------------------------------------------------------------
    # Object model
    #--------------------------------------------------------------------------

    def get(self, attribute_name, default=NEVER_SET, unref=True, resolve=True,
            allow_never_set=True, blocking=True):

        attribute = self.get_attribute(attribute_name)

        if hasattr(attribute, 'func') and attribute.func:
            # Handling Lambda attributes
            value = attribute.func(self)
        else:
            if self.is_local_attribute(attribute.name):
                value = vars(self).get(attribute.name, NEVER_SET)
            else:
                # A pending value has priority
                value = self._state.dirty.get(attribute.name, NEVER_SET)
                if value.value is not NEVER_SET:
                    value = value.value
                else:
                    # otherwise, let's use a previously fetched value if it
                    # exists
                    value = vars(self).get(attribute.name, NEVER_SET)

        if value is NEVER_SET:
            if not allow_never_set:
                log.error(E_GET_NON_LOCAL.format(attribute_name,
                            self._state.uuid))
                raise NotImplementedError

            # node.routing_table is local and auto, so this needs to be tested first...
            if attribute.auto:
                # Automatic instanciation
                #
                # Used for instance in route.node.routing_table.routes
                # XXX We can only auto_instanciate local attributes, otherwise we
                # get issues with asyncio loop not present in thread
                if attribute.requirements:
                    log.warning('Ignored requirements {}'.format(
                                attribute.requirements))
                value = self.auto_instanciate(attribute)

            if value is NEVER_SET:
                if self.is_local_attribute(attribute.name):
                    if attribute.is_collection:
                        value = self.get_default_collection(attribute)
                    else:
                        value = self.get_default(attribute)
                    self.set(attribute.name, value)
                else:
                    # printing self might do an infinite loop here !
                    log.info("Fetching remote value for {}.{}".format(self.get_uuid(), attribute.name))
                    task = getattr(self, "_get_{}".format(attribute.name))()
                    #XXX This is ugly but it prevents the LxdNotFound exception
                    while True:
                        try:
                            rv = task.execute_blocking()
                            break
                        except LXDAPIException:
                            log.warning("LxdAPIException, retrying to fetch value")
                            continue
                        except Exception as e:
                            import traceback; traceback.print_tb(e.__traceback__)
                            log.error("Failed to retrieve remote value for {} on {}".format(attribute.name, self))
                            import os; os._exit(1)
                    value = rv[attribute.name] if isinstance(rv, dict) else rv
                    vars(self)[attribute.name] = value

        if unref and isinstance(value, UUID):
            value = self.from_uuid(value)

        if resolve and isinstance(value, Reference):
            if value._resource is Self:
                value = getattr(self, value._attribute)
            else:
                value = getattr(value._resource, value._attribute)

        return value

    # XXX async_get should not be blocking
    async def async_get(self, attribute_name, default=NEVER_SET, unref=True,
            resolve=True, allow_never_set=False, blocking=True):
        attribute = self.get_attribute(attribute_name)

        # Handling Lambda attributes
        if hasattr(attribute, 'func') and attribute.func:
            value = self.func(self)
        else:
            if self.is_local_attribute(attribute.name):
                value = vars(self).get(attribute.name, NEVER_SET)
            else:

                # A pending value has priority
                value = self._state.dirty.get(attribute.name, NEVER_SET)
                if value.value is not NEVER_SET:
                    value = value.value
                else:
                    # otherwise, let's use a previously fetched value if it
                    # exists
                    value = vars(self).get(attribute.name, NEVER_SET)
                    if value is NEVER_SET:
                        await self._state.manager.attribute_get(self,
                                attribute_name, value)
                        value = vars(self).get(attribute.name, NEVER_SET)

        # Handling NEVER_SET
        if value is NEVER_SET:
            if not allow_never_set:
                log.error(E_GET_NON_LOCAL.format(attribute_name,
                            self._state.uuid))
                raise NotImplementedError

            if attribute.is_collection:
                value = self.get_default_collection(attribute)
            else:
                if attribute.auto:
                    # Automatic instanciation
                    if attribute.requirements:
                        log.warning('Ignored requirements {}'.format(
                                    attribute.requirements))
                    value = self.auto_instanciate(attribute)

                if value is NEVER_SET:
                    value = self.get_default(attribute)

            if value is self.is_local_attribute(attribute.name):
                self.set(attribute.name, value)

        if unref and isinstance(value, UUID):
            value = self.from_uuid(value)

        if resolve and isinstance(value, Reference):
            if value._resource is Self:
                value = getattr(self, value._attribute)
            else:
                value = getattr(value._resource, value._attribute)

        return value

    def _set(self, attribute_name, value, current=False, set_reverse=True):
        """
        Note that set does not automatically mark a resource dirty.
        We might need a flag to avoid dirty by default, which will be useful
        when a resource is modified by another resource: eg x.up, or
        x.ip_address = y, ...
        Returns : task that can be monitored (note that it is not scheduled)
        """
        attribute = self.get_attribute(attribute_name)

        # Let's transform value if not in the proper format
        if attribute.is_collection and not isinstance(value, Collection):
            value = Collection.from_list(value, self, attribute)
        else:
            if isinstance(value, UUID):
                value = self.from_uuid(value)

        # XXX XXX quick fix
        from netmodel.model.type import InetAddress
        if issubclass(attribute.type, InetAddress) and value is not None \
                and not isinstance(value, InetAddress) and not isinstance(value, Reference):
            value = attribute.type(value)

        if set_reverse and attribute.reverse_name:
            for base in self.__class__.mro():
                if not hasattr(base, '_reverse_attributes'):
                    continue

                for ra in base._reverse_attributes.get(attribute, list()):
                    if ra.multiplicity == Multiplicity.OneToOne:
                        if value is not None:
                            value.set(ra.name, self, set_reverse = False)
                    elif ra.multiplicity == Multiplicity.ManyToOne:
                        for element in value:
                            element.set(ra.name, self, set_reverse = False)
                    elif ra.multiplicity == Multiplicity.OneToMany:
                        if value is not None:
                            collection = value.get(ra.name)
                            collection.append(self)
                        else:
                            value is None
                    elif ra.multiplicity == Multiplicity.ManyToMany:
                        # Example:
                        # _set(self, attribute_name)
                        # self = Resource()
                        # attribute_name = <Attribute groups>
                        # value = <Collection 140052309461896 [<Group: topology resources=[], name=topology, owner=None, managed=True>]>
                        # element = <Group: ...>

                        # We add each element of the collection to the remote
                        # attribute which is also a collection
                        for element in value:
                            collection = element.get(ra.name)
                            # XXX duplicates ?
                            collection.append(self)

        return value

    def set(self, attribute_name, value, current=False, set_reverse=True,
            blocking = None):
        value = self._set(attribute_name, value, current=current,
                set_reverse=set_reverse)
        if self.is_local_attribute(attribute_name) or current:
            # super()
            if value is None:
                attribute = self.get_attribute(attribute_name)
            vars(self)[attribute_name] = value

        else:
            self._state.manager.attribute_set(self, attribute_name, value)

    async def async_set(self, attribute_name, value, current=False,
            set_reverse=True, blocking=None):
        """
        Example:
         - setting the ip address on a node's interface

        We need to communicate our intention to the resource manager, which will
        process our request in a centralized fashion, and do the necessary
        steps for us to set the value properly.
        """
        value = self._set(attribute_name, value, current=current,
                set_reverse=set_reverse)
        await self._state.manager.attribute_set_async(self, attribute_name, value)

    def set_many(self, attribute_dict, current=False):
        if not attribute_dict:
            return
        for k, v in attribute_dict.items():
            self.set(k, v, current=current)

    def is_set(self, attribute_name):
        return attribute_name in vars(self)

#    def clean(self, attribute_name):
#        return self._state.manager.attribute_clean(self, attribute_name)

    def is_local_attribute(self, attribute_name):
        ACTIONS = ['get', 'set', 'add', 'remove']
        for action in ACTIONS:
            method = '_{}_{}'.format(action, attribute_name)
            if hasattr(self, method) and getattr(self, method) is not None:
                return False
        return True

    def get_default_collection(self, attribute):
        if isinstance(attribute.default, types.FunctionType):
            default = attribute.default(self)
        elif isinstance(attribute.default, Reference):
            if attribute.default._resource is Self:
                default = getattr(self, attribute.default._attribute)
            else:
                default = getattr(attribute.default._resource,
                        attribute.default._attribute)
        else:
            default = attribute.default
        return Collection.from_list(default, self, attribute)

    def get_default(self, attribute):
        if isinstance(attribute.default, types.FunctionType):
            value = attribute.default(self)
        elif isinstance(attribute.default, Reference):
            if attribute.default._resource is Self:
                value = getattr(self, attribute.default._attribute)
            else:
                value = getattr(attribute.default._resource,
                        attribute.default._attribute)
        else:
            value = copy.deepcopy(attribute.default)
        return value

    def async_get_task(self, attribute_name):
        task = getattr(self, '_get_{}'.format(attribute_name))()
        assert not isinstance(task, tuple)
        return task


    def async_set_task(self, attribute_name, value):
        raise NotImplementedError
        return async_task(async_set_task, attribute_name, value)

    @classmethod
    def get_attribute(cls, key):
        # Searchs if it is a recursive attribute
        try:
            pos = key.find('.')
            if pos >= 0:
                attr, subattr = key[0:pos], key[pos+1: len(key)]
                return getattr(cls,attr).type.get_attribute(subattr)
            return getattr(cls, key)
        except AttributeError:
            return None

#######    @classmethod
#######    def _sanitize(cls):
#######        """
#######        This methods performs sanitization of the object declaration.
#######
#######        More specifically:
#######         - it goes over all attributes and sets their name based on the python
#######           object attribute name.
#######         - it establishes mutual object relationships through reverse attributes.
#######
#######        """
#######        cls._reverse_attributes = dict()
#######        cur_reverse_attributes = dict()
#######        for name, obj in vars(cls).items():
#######            if not isinstance(obj, ObjectSpecification):
#######                continue
#######
#######            # XXX it seems obj should always be an attribute, confirm !
#######            if isinstance(obj, Attribute):
#######                obj.name = name
#######
#######            # Remember whether a reverse_name is defined before loading
#######            # inherited properties from parent
#######            has_reverse = bool(obj.reverse_name)
#######
#######            # Handle overloaded attributes
#######            # By recursion, it is sufficient to look into the parent
#######            for base in cls.__bases__:
#######                if hasattr(base, name):
#######                    parent_attribute = getattr(base, name)
#######                    obj.merge(parent_attribute)
#######                    assert obj.type
#######
#######            # Handle reverse attribute
#######            #
#######            # NOTE: we need to do this after merging to be sure we get all
#######            #   properties inherited from parent (eg. multiplicity)
#######            #
#######            # See "Reverse attributes" section in BaseResource docstring.
#######            #
#######            # Continueing with the same example, let's detail how it is handled:
#######            #
#######            # Original declaration:
#######            # >>>
#######            # class Group(Resource):
#######            #     resources = Attribute(Resource, description = 'Resources belonging to the group',
#######            # 	    multiplicity = Multiplicity.ManyToMany,
#######            #             default = [],
#######            #             reverse_name = 'groups',
#######            #             reverse_description = 'Groups to which the resource belongs')
#######            # <<<
#######            #
#######            # Local variables:
#######            #   cls = <class 'vicn.resource.group.Group'>
#######            #   obj = <Attribute resources>
#######            #   obj.type = <class 'vicn.core.Resource'>
#######            #   reverse_attribute = <Attribute groups>
#######            #
#######            # Result:
#######            #    1) Group._reverse_attributes =
#######            #       { <Attribute resources> : [<Attribute groups>, ...], ...}
#######            #    2) Add attribute <Attribute groups> to class Resource
#######            #    3) Resource._reverse_attributes =
#######            #       { <Attribute groups> : [<Attribute resources], ...], ...}
#######            #
#######            if has_reverse:
#######                a = {
#######                    'name'                  : obj.reverse_name,
#######                    'description'           : obj.reverse_description,
#######                    'multiplicity'          : Multiplicity.reverse(obj.multiplicity),
#######                    'reverse_name'          : obj.name,
#######                    'reverse_description'   : obj.description,
#######                    'auto'                  : obj.reverse_auto,
#######                }
#######                reverse_attribute = Attribute(cls,  **a)
#######                reverse_attribute.is_aggregate = True
#######
#######                # 1) Store the reverse attributes to be later inserted in the
#######                # remote class, at the end of the function
#######                # TODO : clarify the reasons to perform this in two steps
#######                cur_reverse_attributes[obj.type] = reverse_attribute
#######
#######                # 2)
#######                if not obj in cls._reverse_attributes:
#######                    cls._reverse_attributes[obj] = list()
#######                cls._reverse_attributes[obj].append(reverse_attribute)
#######
#######                # 3)
#######                if not reverse_attribute in obj.type._reverse_attributes:
#######                    obj.type._reverse_attributes[reverse_attribute] = list()
#######                obj.type._reverse_attributes[reverse_attribute].append(obj)
#######
#######        # Insert newly created reverse attributes in the remote class
#######        for kls, a in cur_reverse_attributes.items():
#######            setattr(kls, a.name, a)

    @classmethod
    def iter_attributes(cls, aggregates = False):
        for name in dir(cls):
            attribute = getattr(cls, name)
            if not isinstance(attribute, Attribute):
                continue
            if attribute.is_aggregate and not aggregates:
                continue

            yield attribute

    @classmethod
    def iter_keys(cls):
        for name in dir(cls):
            key = getattr(cls, name)
            if not isinstance(key, Key):
                continue
            yield key

    def get_attributes(self, aggregates = False):
        return list(self.iter_attributes(aggregates = aggregates))

    @classmethod
    def has_attribute(cls, name):
        return name in [a.name for a in cls.attributes()]

    def get_attribute_names(self, aggregates = False):
        return set(a.name
                for a in self.iter_attributes(aggregates = aggregates))

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
                    if uuid and isinstance(x, Resource):
                        x = x._state.uuid._uuid
                    ret[a.name].append(x)
            else:
                if uuid and isinstance(value, Resource):
                    value = value._state.uuid._uuid
                ret[a.name] = value
        return ret

    @classmethod
    def get_keys(cls):
        return list(cls.iter_keys())

    @classmethod
    def has_key_attribute(cls, attribute):
        return any(attribute in key for key in cls.iter_keys())

    def auto_instanciate(self, attribute):
        if self.managed is False:
            raise ResourceNotFound(E_AUTO_UNM.format(attribute, self))
        cstr_attributes = dict()

        for a in attribute.type.iter_attributes():
            if not a.mandatory:
                continue

            # Let's find attributes in the remote class that are of my
            # class, and let's setup them to me
            if issubclass(a.type, Resource) and isinstance(self, a.type):
                cstr_attributes[a.name] = self
                continue

            if hasattr(self, a.name):
                cstr_attributes[a.name] = getattr(self, a.name)

        capabilities = set()
        reqs = self._state.manager.get_instance_requirements(self)
        for req in reqs:
            if req._type != attribute.name:
                continue

            for attr_name, prop in req.properties.items():
                value = next(iter(prop.value))
            capabilities |= req._capabilities

        # We need to find a subclass of self._resource with proper capabilities
        cls = self._state.manager.get_resource_with_capabilities(
                attribute.type, capabilities)

        # Before creating a new instance of a class, let's check
        resource = cls(**cstr_attributes)

        self._state.manager.commit_resource(resource)
        return resource

    def get_tuple(self):
        return (self.__class__, self._get_attribute_dict())

    @property
    def state(self):
        return self._state.state

    @state.setter
    def state(self, state):
        self._state.state = state

    def get_types(self):
        return [cls.__name__.lower() for cls in self.__class__.mro()
                if cls.__name__ not in ('ABC', 'BaseType', 'object')]

    def get_type(self):
        return self.__class__.__name__.lower()

    def has_type(self, typ):
        return typ in self.get_types()

    def __repr__(self):
        # Showing aggregate attributes can cause infinite loops
        name = self._state.uuid if self.name in (None, NEVER_SET) else self.name
        return '<{}: {} {}>'.format(self.__class__.__name__, name,
                ', '.join('{}={}'.format(k,v)
                    for k, v in self.get_attribute_dict().items()))

    def __str__(self):
        return self.__repr__()

    def to_dict(self):
        dic = self.get_attribute_dict(aggregates = True)
        dic['id']    = self._state.uuid._uuid
        dic['type']  = self.get_types()
        dic['state'] = self._state.state
        dic['log']   = self._state.log
        return dic

    #---------------------------------------------------------------------------
    # Resource helpers
    #---------------------------------------------------------------------------

    def get_dependencies(self, allow_unresolved = False):
        if not self._deps:
            deps = set()
            for a in self.iter_attributes():
                if not issubclass(a.type, Resource):
                    continue
                if a.is_aggregate:
                    continue

                value = getattr(self, a.name)
                if not value:
                    continue

                if a.multiplicity in (Multiplicity.OneToOne,
                        Multiplicity.ManyToOne):
                    resource = value
                    if not resource:
                        log.warning('Null resource')
                        continue
                    if not resource.managed:
                        continue
                    uuid = resource._state.uuid
                    # Avoid considering oneself as a dependency due to
                    # ResourceAttribute(Self)
                    if uuid != self._state.uuid:
                        deps.add(uuid)
                else:
                    resources = value
                    for cpt, resource in enumerate(resources):
                        if not resource:
                            log.warning('Null resource in collection')
                            continue
                        if not resource.managed:
                            continue
                        uuid = resource._state.uuid
                        deps.add(uuid)
            self._deps = deps
        return self._deps

    def make_name(self, *args, type=True, id=True):
        l = list()
        if type:
            l.append(self.__class__.__name__)
        l.extend(list(args))
        if id:
            N = 3
            uuid = ''.join(random.choice(string.ascii_uppercase +
                        string.digits) for _ in range(N))
            l.append(uuid)
        name = NAME_SEP.join(str(x) for x in l)
        return name

    def check_requirements(self):
        for attr in self.iter_attributes():
            if issubclass(attr.type, Resource) and attr.requirements:
                for req in attr.requirements:
                    instance = getattr(self, attr.name)
                    req.check(instance)

    #--------------------------------------------------------------------------
    # Triggers
    #--------------------------------------------------------------------------

    @deprecated
    def trigger(self, action, attribute_name, *args, **kwargs):
        self._state.manager.trigger(self, action, attribute_name,
                *args, **kwargs)

    #--------------------------------------------------------------------------
    # Object model
    #
    # Only assignment is implemented here, other operators are overloaded in
    # the Attribute class (core.attribute.Attribute)
    #--------------------------------------------------------------------------

    def format(self, fmt):
        return fmt.format(**self.get_attribute_dict(uuid = False))

    def get_tag(self, tag_name, default = NEVER_SET):
        """
        A tag corresponds to a propery that is required by a class in all of
        its inheritors. For instance, a service requires than a subclass
        informs about the 'service_name' tag, which is a class member named
        according to the following convention : __service_name__.
        """
        tag = '__{}__'.format(tag_name)
        if not tag in vars(self.__class__):
            if default is NEVER_SET:
                return default
            raise NotImplementedError('Missing tag {} in class {}'.format(tag,
                        self.__class__.__name__))
        return getattr(self.__class__, tag)

    def iter_backrefs(self):
        for base in self.__class__.mro():
            if not hasattr(base, '_reverse_attributes'):
                continue
            for attr, rattrs in base._reverse_attributes.items():
                instances = self.get(attr.name, allow_never_set = True)
                if instances in (None, NEVER_SET):
                    continue
                if not attr.is_collection:
                    instances = [instances]
                for instance in instances:
                    for rattr in rattrs:
                        yield instance, rattr

    #---------------------------------------------------------------------------
    # Accessors
    #---------------------------------------------------------------------------

    def has_callback(self, action, attribute):
        return hasattr(self, '_{}_{}'.format(action, attribute.name))

    def is_setup(self):
        return self.state in (ResourceState.SETUP_PENDING,
                ResourceState.SETUP, ResourceState.DIRTY)

    __get__ = None
    __create__ = None
    __delete__ = None

#-------------------------------------------------------------------------------
# Helper functions
#-------------------------------------------------------------------------------

# The following Mixin are useful to convert an expresson of subresources into
# an expression of tasks.

class ConcurrentMixin:
    async def async_commit_to_manager(self, manager):
        await asyncio.gather(*[element.async_commit_to_manager(manager)
                for element in self._elements])
        await asyncio.gather(*[e._state.clean.wait() for e in self._elements])
        self._state.clean.set()

class SequentialMixin:
    async def async_commit_to_manager(self, manager):
        for element in self._elements:
            await element.async_commit_to_manager(manager)
            await element._state.clean.wait()
        self._state.clean.set()

class CompositionMixin:
    async def async_commit_to_manager(self, manager):
        for element in self._elements:
            await element.async_commit_to_manager(manager)
            await element._state.clean.wait()
        self._state.clean.set()

_Resource, EmptyResource = SchedulingAlgebra(BaseResource, ConcurrentMixin,
        CompositionMixin, SequentialMixin)

class ManagedResource(_Resource):
    def __init__(self, *args, **kwargs):
        from vicn.core.resource_mgr         import ResourceManager
        owner = kwargs.get('owner', None)
        name = kwargs.get('name', None)

        manager = ResourceManager()
        self.register_to_manager(manager, name=name)

        # Manager is needed for reference and reverse attributes
        super().__init__(*args, **kwargs)

    async def async_commit_to_manager(self, manager):
        if not self.managed:
            return
        self._state.manager.commit_resource(self)

    def register_to_manager(self, manager, name = None):
        if not self.managed:
            return
        manager.add_resource(self, name = name)

Resource = ManagedResource

class BashResource(Resource):
    """
    __get__ : use return code of the bash command

    Intermediate values and attributes: should be dict-like
    Actually, we should collect attributes: dict update/remove, map/reduce
    """
    __node__       = None
    __cmd_get__    = None
    __cmd_create__ = None
    __cmd_delete__ = None

    def __get__(self):
        assert self.__cmd_get__
        return BashTask(self.node, self.__cmd_get__, {'self': self})

    def __create__(self):
        assert self.__cmd_create__
        return BashTask(self.node, self.__cmd_create__, {'self': self})

    def __delete__(self):
        assert self.__cmd_delete__
        return BashTask(self.node, self.__cmd_delete__, {'self': self})

