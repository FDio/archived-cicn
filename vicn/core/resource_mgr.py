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

import sys, logging, asyncio, socket, functools
import time

# LXD workaround
from pylxd.exceptions import NotFound as LxdNotFound, LXDAPIException

from netmodel.model.collection  import Collection
from netmodel.model.filter      import Filter
from netmodel.model.query       import Query, ACTION_SELECT, ACTION_INSERT
from netmodel.model.query       import ACTION_UPDATE, ACTION_SUBSCRIBE
from netmodel.model.sql_parser  import SQLParser
from netmodel.network.packet    import Packet
from netmodel.network.router    import Router
from netmodel.util.toposort     import toposort, toposort_flatten
from netmodel.util.meta         import inheritors
from netmodel.util.singleton    import Singleton
from netmodel.util.misc         import is_iterable
from vicn.core.attribute        import NEVER_SET
from vicn.core.commands         import ReturnValue
from vicn.core.exception        import VICNException, ResourceNotFound
from vicn.core.resource_factory import ResourceFactory
from vicn.core.resource         import Resource, FactoryResource, EmptyResource
from vicn.core.state            import InstanceState, ResourceState
from vicn.core.state            import AttributeState, Operations, PendingValue
from vicn.core.task             import TaskManager, wait_task, task, async_task
from vicn.core.task             import EmptyTask, BashTask

log = logging.getLogger(__name__)

# NOTE: Do not fully reinitialize a resource after a step fails since it will
# call initialize several times, and might created spurious resources.
ENABLE_LXD_WORKAROUND = False
DEFAULT_QTPLAYER_PORT = 8999

# Monitoring queries

Q_SUB_VPP = 'SUBSCRIBE SUM(*) FROM interface WHERE device_name INCLUDED [{}]'
Q_SUB_IF = 'SUBSCRIBE * FROM interface WHERE device_name == "{}"'
Q_SUB_VPP_IF = 'SUBSCRIBE * FROM vpp_interface WHERE device_name == "{}"'
Q_SUB_STATS = 'SUBSCRIBE * FROM stats'
Q_SUB_EMULATOR_IF = 'SUBSCRIBE * FROM interface WHERE id == "{}"'
Q_SUB_EMULATOR = 'SUBSCRIBE * FROM interface WHERE device_name == "{}"'

# Log messages

S_WAIT_DEP = '     .. Waiting for dependency      {}'
S_WAIT_DEP_OK = '     .. Done waiting for dependency {}'
S_WAIT = '     .. Waiting for      {}'
S_WAIT_OK = '     .. Done waiting for {}'
S_WAIT_PRED = ' - Waiting for initialization of predecessors...'
S_WAIT_SRS = ' - Waiting for subresources...'
S_REG_SR = '  . Registering subresource to manager {}...'
S_WAIT_SR = '  . Waiting for subresource: {}'
S_WAIT_SR_OK = '  . Subresource is ready: {}'
S_AFTER = '  . AFTER TYPE={}'

S_INIT_DONE = 'INIT done. Resource exists. Process attribute dict {}'
S_GET_DONE = 'GET done. Resource does not exist (exception was: {})'
S_KEYS_OK = 'Keys initialized, resource can now be created.'
S_CREATE_OK = 'CREATE success. Process attribute dict {}'

#------------------------------------------------------------------------------
# Helpers
#------------------------------------------------------------------------------

async def wait_resource(resource):
    await resource._state.clean.wait()

async def wait_resource_init(resource):
    await resource._state.init.wait()

async def wait_resources(resources):
    await asyncio.gather(*[wait_resource(r) for r in resources])

wait_resource_task = async_task(wait_resource)
wait_resources_task = async_task(wait_resources)

#------------------------------------------------------------------------------

class ResourceManager(metaclass=Singleton):
    """
    A ResourceManager is in charge of managing resources, their lifecycle, and
    interfaces to them.
    """

    def __init__(self, base, settings):

        # Base directory for scenario
        self._base = base

        # Resources sorted via dependency (instances)
        self._resources         = dict()

        self._deps = None

        # Store resource requirements used for automatic instanciation
        # instance -> attribute -> requirements
        self._instance_requirements = dict()

        self._dirty = set()
        self._auto_commit = False

        # class -> Requirements
        self._class_requirements = dict()

        self._map_uuid_name = dict()
        self._map_name_uuid = dict()
        self._map_str_uuid = dict()

        # The task manager is used to schedule tasks used for resource
        # synchronization
        self._task_mgr = TaskManager()

        # Store experiment settings
        self._settings = settings

        # Cache available resource types
        _available = ResourceFactory().get_available_resources()
        self._available = { k.lower(): v for k, v in _available.items() }

        # API / user interface
        self._router = Router(vicn_callback = self._on_vicn_command)
        self._router.add_interface('unixserver')
        self._router.add_interface('local', router = self._router)
        self._router.add_interface('vicn', manager = self)

        ws_port = self.get('websocket_port')
        self._ws = self._router.add_interface('websocketserver',
                port = ws_port)

        # Monitoring
        self._monitored = set()
        self._pending_monitoring = set()
        self._map_ip_interface = dict()
        self._monitored_channels = set()

        # For debug
        self._committed = set()

        self._num = 0
        self._num_clean = 0

    def terminate(self):
        self._router.terminate()

    #--------------------------------------------------------------------------
    # Settings
    #--------------------------------------------------------------------------

    def set_settings(self, settings):
        if settings is None:
            return
        self._settings.update(settings)

    def get(self, setting):
        return self._settings[setting]

    def set(self, setting, value):
        self._settings[setting] = value

    #--------------------------------------------------------------------------
    # Monitoring
    #
    # XXX This code should be deprecated / moved into a separate module.
    # Planned for a future release.
    #--------------------------------------------------------------------------

    def _on_vicn_command(self, command):
        if command == 'setup':
            self.setup()
        elif command == 'teardown':
            self.teardown()
        elif command == 'monitor':
            self.monitor()
        elif command == 'terminate':
            loop = asyncio.get_event_loop()
            loop.stop()
        else:
            # open_terminal, ...
            raise NotImplementedError

    def _broadcast(self, query):
        if not self._ws:
            return
        self._ws.execute(query)

    def _broadcast_packet(self, packet):
        self._broadcast(packet.to_query())

    def _on_qtplayer_packet(self, name, packet):
        query = packet.to_query()
        query.params['name'] = name
        query.reply = True
        self._ws.execute(query)
        return None

    def _on_ns_record(self, packet):
        query = packet.to_query()

        if not query.object_name == 'interface':
            return
        q = Query(ACTION_UPDATE, 'channel', filter = query.filter,
                params = query.params)
        q.reply = True

        self._ws.execute(q)
        return None

    def _on_netmon_record(self, packet):
        query = packet.to_query()

        # Find channel related to query
        # NOTE: we update the channel twice, once for each interface...
        if query.object_name == 'interface':
            device_names = [value for key, op, value in query.filter.to_list()
                    if key == 'device_name']
            if not device_names:
                log.error('No device name in packet=', packet)
                return
            device_name = device_names[0]
            node_name = query.params['node']
            node = ResourceManager().by_name(node_name)
            if node is None:
                return None
            for interface in node.interfaces:
                if interface.device_name == device_name:
                    if interface.channel:
                        f = Filter.from_list([['id', '==',
                                interface.channel._state.uuid._uuid]])
                        q = Query(ACTION_UPDATE, 'channel', filter = f,
                                params = query.params)
                        q.reply = True
                        self._ws.execute(q)
                        return None
                    return None
            return None
        elif query.object_name == 'vpp_interface':
            device_names = [value for key, op, value in query.filter.to_list()
                    if key == 'device_name']
            if not device_names:
                log.error('No device name in packet=', packet)
                return
            device_name = device_names[0]
            node_name = query.params['node']
            node = ResourceManager().by_name(node_name)
            if node is None:
                print("no node")
                return None
            for interface in node.interfaces:
                if not hasattr(interface, 'vppinterface') or not interface.vppinterface:
                    continue
                if interface.vppinterface.device_name == device_name:
                    if interface.channel:
                        f = Filter.from_list([['id', '==',
                                interface.channel._state.uuid._uuid]])
                        q = Query(ACTION_UPDATE, 'channel', filter = f,
                                params = query.params)
                        q.reply = True
                        self._ws.execute(q)
                        return None
                    print("no channel")
                    return None
            print("no vpp interface found")
            return None

        return None

    def _on_netmon_channel_record(self, packet):
        query = packet.to_query()
        if query.object_name == 'interface':
            device_names = [value for key, op, value in query.filter.to_list()
                    if key == 'device_name']
            if not device_names:
                log.error('No device name in packet=', packet)
                return

            device_name = device_names[0]

            f = Filter.from_list([['id', '==', device_name]])
            q = Query(ACTION_UPDATE, 'channel', filter = f,
                    params = query.params)
            q.reply = True
            self._ws.execute(q)
            return None

        return None

    def _on_vpp_record(self, packet, pylink_id):
        query = packet.to_query()
        if query.object_name == 'interface':
            device_names = [value for key, op, value in query.filter.to_list()
                    if key == 'device_name']
            if not device_names:
                log.error('No device name in packet=', packet)
                return

            # We might want to check if the query has SUM(*)
            f = Filter.from_list([['id', '==', pylink_id]])
            q = Query(ACTION_UPDATE, 'channel', filter = f,
                    params = query.params)
            q.reply = True
            self._ws.execute(q)
            return None

        print('discard packet in on_netmon_channel_record', query)
        return None

    #--------------------------------------------------------------------------
    # Resource management
    #--------------------------------------------------------------------------

    def create_from_dict(self, **resource):
        resource_type = resource.pop('type', None)

        assert resource_type

        return self.create(resource_type.lower(), **resource)

    def create(self, resource_type, **attributes):
        cls = self._available.get(resource_type)
        if not cls:
            raise Exception("Ignored resource with unknown type %s: %r" %
                    (resource_type, attributes))

        resource = cls(**attributes)

        name = attributes.get('name', None)
        if name:
            self._map_uuid_name[resource._state.uuid] = name
            self._map_name_uuid[name] = resource._state.uuid

        return resource

    def get_resource_type_names(self):
        return ResourceFactory().get_available_resources().keys()

    def add_resource(self, instance, name = None):
        instance._state = InstanceState(self, instance, name = name)

        self._resources[instance._state.uuid] = instance
        self._map_str_uuid[instance._state.uuid._uuid] = instance._state.uuid
        self._deps = None

    def commit_resource(self, resource):
        """
        Committing a resource creates an asyncio function implementing a state
        management automaton.
        """
        self._num += 1
        asyncio.ensure_future(self._process_resource(resource))

    def commit(self):
        """
        Commit all resource whose owner is not set, and mark unmanaged
        resources as clean.

        This function is used at initialization.
        """

        # Start FSM for all managed resources
        for resource in self.get_resources():
            if resource.owner is not None:
                continue
            if resource.managed == False:
                asyncio.ensure_future(self._set_resource_state(resource,
                            ResourceState.CLEAN))
                continue

            self.commit_resource(resource)

    def setup(self, commit=False):
        """
        This function is in charge of setting up all resources needed by the
        experiment. Since it might be a long process, it should be asynchronous
        at some point. So far, we let resources take care of this by themselves.
        """
        self._auto_commit = commit
        if commit:
            self.commit()

    def teardown(self):
        asyncio.ensure_future(self._teardown())

    async def _teardown(self):
        task = EmptyTask()
        # XXX we should never have to autoinstanciate
        # XXX why keeping this code
        for resource in self.get_resources():
            if resource.get_type() == 'lxccontainer':
                task = task | resource.__delete__()
        self.schedule(task)
        ret = await wait_task(task)
        return ret

    def get_resource_with_capabilities(self, cls, capabilities):
        if '__type__' in cls.__dict__ and cls.__type__ == FactoryResource:
            candidates = inheritors(cls)
            if not candidates:
                log.error('Abstract resource with no candidates: %s',
                        cls.__name__)
                return None

            for delegate in candidates:
                if capabilities and (not '__capabilities__' in vars(delegate)
                        or not capabilities.issubset(delegate.__capabilities__)):
                    continue
                log.info("Abstract resource %s, delegated %s among %r" % \
                        (cls.__name__, delegate.__name__, candidates))
                return delegate
            return None
        else:
            if capabilities and (not '__capabilities__' in vars(delegate) or
                    not capabilities.issubset(delegate.__capabilities__)):
                log.error('Capabilities conflict for resource : %s',
                        cls.__name__)
                raise VICNException
            return cls

    def find(self, resource_tuple):
        cls, attr_dict = resource_tuple
        for instance in self.by_type(cls):
            cur_attr_dict = instance._get_attribute_dict()
            common_keys = [k for k in cur_attr_dict.keys()
                    if k in attr_dict.keys()]
            if all(attr_dict[k] == cur_attr_dict[k] for k in common_keys):
                return instance
        return None

    def __iter__(self):
        for resource in self._resources.values():
            yield resource

    def resources(self):
        return list(self.__iter__())

    def _sort_resources(self):
        deps = {}
        for instance in self.resources():
            deps[instance._state.uuid] = \
                    instance.get_dependencies(allow_unresolved = True)

        self._deps = toposort_flatten(deps)

    def _sorted_resources(self):
        """
        Iterates on resources based on their dependencies
        """
        if not self._deps:
            self._sort_resources()
        for dep in self._deps:
            try:
                yield self._resources[dep]
            except KeyError:
                log.error('Dependency not found : {}'.format(dep))
                raise InvalidResource

    def sorted_resources(self):
        return list(self._sorted_resources())

    #--------------------------------------------------------------------------
    # Queries
    #--------------------------------------------------------------------------

    def by_uuid(self, uuid):
        return self._resources.get(uuid)

    def by_uuid_str(self, uuid_str):
        uuid = self._map_str_uuid.get(uuid_str)
        return self._resources.get(uuid)

    def by_name(self, name):
        uuid = self._map_name_uuid.get(name)
        return self.by_uuid(uuid)

    def get_resources(self):
        return self._resources.values()

    def get_aggregates(self, resource_name, resource_cls):
        """
        Get aggregated object.
        """
        if not resource_name in self._aggregates:
            return None
        all_aggregates = self._aggregates[resource_name]

        if not resource_cls in all_aggregates:
            return None
        aggregates = all_aggregates[resource_cls]

        assert all(isinstance(x, resource_cls) for x in aggregates)
        return aggregates

    def get_aggregate(self, resource_name, resource_cls):
        aggregates = self.get_aggregates(resource_name, resource_cls)
        if not aggregates:
            return None
        assert len(aggregates) == 1
        return next(aggregates)

    def by_type(self, type):
        return [r for r in self if isinstance(r, type)]

    def by_type_str(self, typestr):
        cls = self._available.get(typestr.lower())
        if not cls:
            return list()
        return self.by_type(cls)

    #--------------------------------------------------------------------------
    # Requirements
    #--------------------------------------------------------------------------

    def add_instance_requirement(self, instance, requirement):
        uuid = instance._state.uuid
        if not uuid in self._instance_requirements:
            self._instance_requirements[uuid] = list()
        self._instance_requirements[uuid].append(requirement)

    def get_instance_requirements(self, instance):
        uuid = instance._state.uuid
        return self._instance_requirements.get(uuid, dict())

    def add_class_requirement(self, cls, requirement):
        if not cls in self._class_requirements:
            self._class_requirements[cls] = list()
        self._class_requirements[cls].append(requirement)

    #--------------------------------------------------------------------------
    # Events
    #--------------------------------------------------------------------------

    def on(self, resource_name, event, action):
        resource = self._resources.get(resource_name, None)
        if not resource:
            return

        resource.on(event, action)

    #--------------------------------------------------------------------------
    # Task management
    #--------------------------------------------------------------------------

    def schedule(self, task, resource = None):
        if task is None or isinstance(task, EmptyTask):
            return
        self._task_mgr.schedule(task, resource)

    #--------------------------------------------------------------------------
    # Asynchronous resource API
    #
    # The manager is the only one to submit tasks to the scheduler since it can
    # store and share the results, manage concurrent access, etc.
    # As many functions are not thread safe, we make sure that they are all
    # executed in the manager's thread (=main thread).
    #--------------------------------------------------------------------------

    async def resource_exits(self, resource):
        await self._resource_get()
        await self.wait_resource_exists(resource)
        return resource._state.exists

    async def wait_attr_init(self, resource, attribute_name):
        await resource._state.attr_init[attribute_name].wait()

    async def wait_attr_clean(self, resource, attribute_name):
        await resource._state.attr_clean[attribute_name].wait()

    async def attribute_get(self, resource, attribute, value):
        await self.wait_attr_init(resource, attribute)
        return resource.get(attribute)

    async def _attribute_set(self, resource, attribute_name, value):
        with await resource._state.write_lock:

            attr_state = resource._state.attr_state[attribute_name]
            if attr_state == AttributeState.CLEAN:
                resource._state.attr_state[attribute_name] = \
                        AttributeState.DIRTY
            elif attr_state in [
                # Nothing to do since we know the attribute value will be
                # processed later.
                # If the attribute was not processed by default, we would have
                # to change the state of the attribute so that it gets
                # processed.
                AttributeState.UNINITIALIZED,
                AttributeState.INITIALIZED,
                AttributeState.PENDING_INIT,
                AttributeState.DIRTY]:
                pass
            else:
                # We cannot have the lock for instance if the attribute is
                # being updated.
                raise RuntimeError

            resource_state = resource._state.state
            if resource_state == ResourceState.CLEAN:
                resource._state.state = ResourceState.DIRTY
                resource._state.change_event.set()
            elif resource_state in [
                ResourceState.UNINITIALIZED,
                ResourceState.INITIALIZED,
                ResourceState.PENDING_KEYS,
                ResourceState.KEYS_OK,
                ResourceState.PENDING_DEPS,
                ResourceState.DEPS_OK,
                ResourceState.PENDING_CREATE,
                ResourceState.CREATED,
                ResourceState.DIRTY,
            ]:
                pass # Nothing to do, the attribute will get processed
            else:
                # ResourceState.PENDING_UPDATE
                # other
                raise RuntimeError("Resource cannot be in state".format(
                            resource_state))

#        if blocking:
#            await self.wait_attr_clean(resource, attribute_name)

    def attribute_set(self, resource, attribute_name, value):
        # Add the current operation to the pending list
        # NOTE: collections are unordered and can be updated concurrently
        #self._attribute_set_pending_value(resource, attribute_name)
        resource._state.dirty[attribute_name].trigger(Operations.SET,
                value)
        asyncio.ensure_future(self._attribute_set(resource, attribute_name, value))

    async def attribute_set_async(self, resource, attribute_name, value):
        # Add the current operation to the pending list
        # NOTE: collections are unordered and can be updated concurrently
        #self._attribute_set_pending_value(resource, attribute_name)
        resource._state.dirty[attribute_name].trigger(Operations.SET,
                value)
        await self._attribute_set(resource, attribute_name, value)



    #---------------------------------------------------------------------------
    # Resource dependency management
    #---------------------------------------------------------------------------

    async def _resource_wait_attributes(self, resource):
        """Check dependencies and requirements

        Inspect all attributes for referenced resources, and their eventual
        requirements.
        """
        self.log(resource, ' - Waiting for attribute dependencies...')
        for attr in resource.iter_attributes():

            if not issubclass(attr.type, Resource):
                continue

            deps = resource.get(attr.name)
            if not deps:
                continue
            if not attr.is_collection:
                deps = [deps]

            for dep in deps:
                if resource.has_key_attribute(attr):
                    if not dep.managed:
                        continue
                    dep_pfx = '{}:{}'.format(dep.get_type(), dep.get_uuid())
                    self.log(resource, S_WAIT_DEP. format(dep_pfx))
                    await wait_resource(dep)
                    self.log(resource, S_WAIT_DEP_OK. format(dep_pfx))

                if not attr.requirements:
                    continue

                for req in attr.requirements:
                    dep_attr_name = req.requirement_type
                    dep_attr = dep.get_attribute(dep_attr_name)
                    assert issubclass(dep_attr.type, Resource)
                    dep_attr_value = dep.get(dep_attr_name)

                    if not dep_attr_value:
                        dep_attr_value = dep.auto_instanciate(dep_attr)
                        setattr(dep, dep_attr_name, dep_attr_value)

                    dep_attr_value_pfx = '{}:{}'.format(
                            dep_attr_value.get_type(),
                            dep_attr_value.get_uuid())
                    self.log(resource,
                            S_WAIT_DEP.format(dep_attr_value_pfx))
                    await wait_resource(dep_attr_value)
                    self.log(resource,
                            S_WAIT_DEP_OK .format(dep_attr_value_pfx))

    async def _resource_wait_predecessors(self, resource):
        after = resource.__after__()
        if after:
            self.log(resource, ' - Waiting for predecessors...')
            for resource_type in after:
                self.log(resource, '  . AFTER TYPE={}'.format(resource_type))
                befores = resource._state.manager.by_type_str(resource_type)
                for before in befores:
                    if not before.managed:
                        continue
                    before_pfx = '{}:{}'.format(before.get_type(),
                            before.get_uuid())
                    self.log(resource, S_WAIT.format(before_pfx))
                    await wait_resource(before)
                    self.log(resource, S_WAIT_OK.format(before_pfx))

        after_init = resource.__after_init__()
        if after_init:
            self.log(resource, S_WAIT_PRED)
            for resource_type in after_init:
                self.log(resource, S_AFTER.format(resource_type))
                befores = resource._state.manager.by_type_str(resource_type)
                for before in befores:
                    if not before.managed:
                        continue
                    before_pfx = '{}:{}'.format(before.get_type(),
                            before.get_uuid())
                    self.log(resource, S_WAIT.format(before_pfx))
                    await wait_resource_init(before)
                    self.log(resource, S_WAIT_OK.format(before_pfx))

    async def _resource_wait_subresources(self, resource):
        self.log(resource, S_WAIT_SRS)

        sr = resource.__subresources__()
        if sr is not None and not isinstance(sr, EmptyResource):
            resource.set_subresources(sr)
            pfx_sr = '{}:{}'.format(sr.get_type(), sr.get_uuid())
            self.log(resource, S_REG_SR .format(pfx_sr))
            await sr.async_commit_to_manager(self)
            self.log(resource, S_WAIT_SR.format(pfx_sr))
            await wait_resource(sr)
            self.log(resource, S_WAIT_SR_OK.format(pfx_sr))

    async def _resource_wait_dependencies(self, resource):
        self.log(resource, 'Waiting for dependencies...')
        await self._resource_wait_attributes(resource)
        await self._resource_wait_predecessors(resource)
        await self._resource_wait_subresources(resource)

    def _task_resource_action(self, resource, action):
        """Perform action: __get__, __create__, __delete__ on the full class
        hierarchy.
        """
        method = getattr(resource, action, None)
        return method() if method else EmptyTask()

    #--------------------------------------------------------------------------
    # Resource model
    #--------------------------------------------------------------------------

    def _task_attribute_op(self, resource, attribute, op):
        return getattr(resource, '_{}_{}'.format(op, attribute.name))()

    #--------------------------------------------------------------------------
    # Attribute FSM
    #--------------------------------------------------------------------------

    def _attribute_is_dirty(self, resource, attribute):
        """
        Precondition:
            Attribute has been retrieved
        """
        pending_value = resource._state.dirty[attribute.name]
        return pending_value.value != NEVER_SET

    async def __set_attribute_state(self, resource, attribute_name, state):
        """Sets the resource state (no-lock version)

        It is important to centralize state change since some states are
        associated with Events().
        """
        if state in [
            AttributeState.INITIALIZED,
            AttributeState.CLEAN,
            AttributeState.DIRTY
        ]:
            resource._state.attr_init[attribute_name].set()
        elif state == AttributeState.RESET:
            resource._state.attr_init[attribute_name].clear()
        else:
            raise RuntimeError("Inconsistent resource state {}".format(state))

        if state in [AttributeState.CLEAN]:
            resource._state.attr_clean[attribute_name].set()
        elif state in [
            AttributeState.INITIALIZED,
            AttributeState.DIRTY,
            AttributeState.RESET
        ]:
            resource._state.attr_clean[attribute_name].clear()
        else:
            raise RuntimeError

        resource._state.attr_state[attribute_name] = AttributeState.UNINITIALIZED \
                if state == AttributeState.RESET else state

    async def _set_attribute_state(self, resource, attribute_name, state):
        """Sets the attribute state (lock version)
        """
        with await resource._state.attr_lock[attribute_name]:
            await self.__set_attribute_state(resource, attribute_name, state)

    def _trigger_attr_state_change(self, resource, attribute, fut):
        try:
            ret = fut.result()
            resource._state.attr_change_success[attribute.name] = True
            resource._state.attr_change_value[attribute.name]  = ret
        except ResourceNotFound as e:
            resource._state.attr_change_success[attribute.name] = False
            resource._state.attr_change_value[attribute.name]  = e
        except Exception as e:
            import traceback; traceback.print_exc()
            resource._state.attr_change_success[attribute.name] = False
            resource._state.attr_change_value[attribute.name]  = e
        resource._state.attr_change_event[attribute.name].set()

    async def attribute_process(self, resource, attribute):
        """
        Temporary FSM executing in parallel for attribute management. Those FSM
        are under the responsability of the main resource FSM.

        Precondition:
            Attribute state is initialized
        """
        self.attr_log(resource, attribute,
                'Starting attribute FSM for {}'.format(attribute.name))

        new_state = None
        while new_state != AttributeState.CLEAN:
            #with await resource._state.attr_lock[attribute.name]:
            state = resource._state.attr_state[attribute.name]
            self.attr_log(resource, attribute,
                    'Current state is {}'.format(state))

            # AttributeState.ERROR
            if resource._state.attr_change_success[attribute.name] == False:
                e = resource._state.attr_change_value[attribute.name]
                if ENABLE_LXD_WORKAROUND and \
                        (isinstance(e, LxdNotFound) or isinstance(e, LXDAPIException)):
                    new_state = AttributeState.RESET
                    log.warning('LXD Fix (not found). Reset attribute')
                    resource._state.attr_change_success[attribute.name] = True
                else:
                    log.error('Attribute error {} for resource {}'.format(
                        attribute.name, resource.get_uuid()))
                    sys.stdout.flush()

                    import traceback; traceback.print_tb(e.__traceback__)
                    log.error('Failed with exception: {}'.format(e))
                    import os; os._exit(1)

                    # Signal update errors to the parent resource
                    resource._state.attr_change_event[attribute.name].set()

            if state == AttributeState.UNINITIALIZED:
                pending_state = AttributeState.PENDING_INIT
            elif state in AttributeState.INITIALIZED:
                pending_state = AttributeState.PENDING_UPDATE
            elif state == AttributeState.DIRTY:
                pending_state = AttributeState.PENDING_UPDATE
            elif state in [
                AttributeState.PENDING_INIT,
                AttributeState.PENDING_UPDATE
            ]:
                # Nothing to do
                pending_state = None
            elif state == AttributeState.CLEAN:
                return
            else:
                raise RuntimeError

            if pending_state is None:
                self.attr_log(resource, attribute,
                        'Nothing to do. Waiting for event...')
                await resource._state.attr_change_event[attribute.name].wait()
                resource._state.attr_change_event[attribute.name].clear()
                self.attr_log(resource, attribute, 'Wake up from event')
                continue

            if pending_state == AttributeState.PENDING_INIT:
                task = self._task_attribute_op(resource, attribute, 'get')
            elif pending_state == AttributeState.PENDING_UPDATE:
                pending_value = resource._state.dirty[attribute.name]

                if pending_value.value == NEVER_SET:
                    assert len(pending_value.operations) == 0
                    task = EmptyTask()
                else:
                    try:
                        task = self._task_attribute_op(resource, attribute,
                                Operations.SET)
                    except Exception as e:
                        log.warning('No attribute setter attribute {}'.format(
                                    attribute))
                        task = EmptyTask()
            else:
                raise RuntimeError

            if task is not None and not isinstance(task, EmptyTask):
                state_change = functools.partial( \
                        self._trigger_attr_state_change, resource, attribute)
                task.add_done_callback(state_change)
                self.attr_log(resource, attribute,
                        'Trigger {} -> {}. Waiting task completion'.format(
                            state, pending_state))
                self.schedule(task)

                await resource._state.attr_change_event[attribute.name].wait()
                resource._state.attr_change_event[attribute.name].clear()
                self.attr_log(resource, attribute,
                        'Completed {} -> {}. Success = {}'.format(
                            state, pending_state,
                            resource._state.attr_change_success[attribute.name]))
            else:
                # If this value is not reset, attributes get updated many times
                resource._state.attr_change_value[attribute.name] = NEVER_SET

            if pending_state == AttributeState.PENDING_INIT:
                if resource._state.attr_change_success[attribute.name] == True:
                    attrs = resource._state.attr_change_value[attribute.name]
                    self.attr_log(resource, attribute,
                            'INIT success. Value = {}'.format(attrs))
                    if not isinstance(attrs, ReturnValue):
                        found = self._process_attr_dict(resource, attribute, attrs)
                    else:
                        found = self._process_attr_dict(resource, attribute, attrs.stdout)
                    if not found:
                        log.error('Attribute missing return attrs: {}'.format(
                                    attrs))
                        found = self._process_attr_dict(resource, attribute,
                                attrs)
                    new_state = AttributeState.INITIALIZED
                else:
                    attrs = resource._state.attr_change_value[attribute.name]
                    if ENABLE_LXD_WORKAROUND and \
                            (isinstance(attrs, LxdNotFound) or isinstance(attrs, LXDAPIException)):
                        new_state = AttributeState.RESET
                        log.warning('LXD Fix (not found). Reset attribute')
                        resource._state.attr_change_success[attribute.name] = True
                    else:
                        self.attr_log(resource, attribute,
                                'INIT gave no value. Value = {}'.format(attrs))
                        new_state = AttributeState.INITIALIZED

            elif pending_state == AttributeState.PENDING_UPDATE:
                attrs = resource._state.attr_change_value[attribute.name]
                if resource._state.attr_change_success[attribute.name] == True:
                    self.attr_log(resource, attribute,
                            'UPDATE success. Value = {}. Attribute is CLEAN'.format(attrs))
                    if not isinstance(attrs, ReturnValue) and attrs != NEVER_SET:
                        # None could be interpreted as the return value. Also,
                        # we need not to overwrite the value from get
                        self._process_attr_dict(resource, attribute, attrs)

                        # We might do this for all returned attributes
                        cur_value = vars(resource)[attribute.name]
                        if attribute.is_collection:
                            tmp = Collection(pending_value.value)
                            tmp._attribute = cur_value._attribute
                            tmp._instance = cur_value._instance
                        else:
                            tmp = pending_value.value
                        vars(resource)[attribute.name] = tmp
                        pending_value.clear()

                    new_state = AttributeState.CLEAN
                else:
                    if ENABLE_LXD_WORKAROUND and \
                            (isinstance(attrs, LxdNotFound) or isinstance(attrs, LXDAPIException)):
                        new_state = AttributeState.RESET
                        log.warning('LXD Fix (not found). Reset attribute')
                        resource._state.attr_change_success[attribute.name] = True
                    else:
                        log.error('Attribute error {} for resource {}'.format(
                            attribute.name, resource.get_uuid()))
                        # XXX need better logging
                        sys.stdout.flush()
                        e = resource._state.attr_change_value[attribute.name]
                        import traceback; traceback.print_tb(e.__traceback__)
                        new_state = AttributeState.ERROR
                        import os; os._exit(1)

            else:
                raise RuntimeError

            # Setting attribute state
            await self._set_attribute_state(resource, attribute.name,
                    new_state)

    #--------------------------------------------------------------------------
    # Resource FSM
    #--------------------------------------------------------------------------

    def parse_query(self, line):
        dic = SQLParser().parse(line)
        if not dic:
            raise RuntimeError("Can't parse input command: %s" % command)

        return Query.from_dict(dic)

    def _monitor_qtplayer(self, resource):
        try:
            ip = resource.node.hostname
        except:
            ip = str(resource.node.management_interface.ip4_address)

        hook = functools.partial(self._on_qtplayer_packet, resource.node.name)
        ws = self._router.add_interface('websocketclient', address=ip,
                port = DEFAULT_QTPLAYER_PORT,
                hook = hook)
        q_str = 'SUBSCRIBE * FROM stats'
        q = self.parse_query(q_str)
        packet = Packet.from_query(q)
        self._router._flow_table.add(packet, None, set([ws]))
        ws.send(packet)

    def _monitor_netmon(self, resource):
        print("MONITOR NODE", resource.node)
        ip = str(resource.node.management_interface.ip4_address)
        if not ip:
            log.error('IP of monitored Node {} is None'.format(resource.node))
            import os; os._exit(1)

        ws = self._router.add_interface('websocketclient', address=ip,
                hook=self._on_netmon_record)

        node = resource.node
        for interface in node.interfaces:
            if not interface.monitored:
                print("non monitored interface", interface)
                continue
            print("NETMON MONITOR INTERFACE", interface)

#            if interface.get_type() == 'dpdkdevice' and hasattr(node,'vpp'):
#
#                # Check if vICN has already subscribed for one interface in
#                # the channel
#                if hasattr(interface.channel,'already_subscribed'):
#                    continue
#
#                channel_id = interface.channel._state.uuid._uuid
#
#                update_vpp = functools.partial(self._on_vpp_record,
#                        pylink_id = channel_id)
#                ws_vpp = self._router.add_interface('websocketclient',
#                        address=ip, hook=update_vpp)
#
#                aggregate_interfaces = list()
#                for _interface in node.interfaces:
#                    if not _interface.get_type() == 'dpdkdevice' and  \
#                            _interface.monitored:
#                        aggregate_interfaces.append('"' +
#                                _interface.device_name + '"')
#
#                q_str = Q_SUB_VPP.format(','.join(aggregate_interfaces))
#                q = self.parse_query(q_str)
#                packet = Packet.from_query(q)
#                self._router._flow_table.add(packet, None, ws_vpp)
#                ws_vpp.send(packet)
#
#                # Prevent vICN to subscribe to other interfaces of the same
#                # channel
#                interface.channel.already_subscribed = True
#
#            else:
            if hasattr(node, 'vpp') and node.vpp is not None:
                q_str = Q_SUB_VPP_IF.format(interface.vppinterface.device_name)
            else:
                q_str = Q_SUB_IF.format(interface.device_name)
            log.warning(" -- MONITOR {}".format(q_str))
            q = self.parse_query(q_str)
            packet = Packet.from_query(q)
            self._router._flow_table.add(packet, None, set([ws]))
            ws.send(packet)

    def _monitor_vpp_interface(self, vpp_interface):
        print("MONITOR interface", vpp_interface)
        interface = vpp_interface.parent
        node = interface.node
        # XXX only monitor in the topology group
        if node.get_type() != 'lxccontainer':
            print("MONITOR  -> Ignored: not in container")
            return

        # We only monitor interfaces to provide data for wired channels
        channel = interface.channel
        if channel is None:
            print("MONITOR  -> Ignored: no channel")
            return
        if channel.has_type('emulatedchannel'):
            print("MONITOR  -> Ignored: belong to wireless channel")
            return

        # Don't monitor multiple interfaces per channel
        if channel in self._monitored_channels:
            print("MONITOR  -> Ignored: channel already monitored")
            return
        self._monitored_channels.add(channel)

        ip = str(node.management_interface.ip4_address)
        if not ip:
            log.error('IP of monitored Node {} is None'.format(resource.node))
            import os; os._exit(1)

        # Reuse existing websockets
        ws = self._map_ip_interface.get(ip)
        if not ws:
            ws = self._router.add_interface('websocketclient', address=ip,
                    hook=self._on_netmon_record)
            self._map_ip_interface[ip] = ws

        q_str = Q_SUB_VPP_IF.format(vpp_interface.device_name)
        print("MONITOR -> query= {}".format(q_str))
        q = self.parse_query(q_str)
        packet = Packet.from_query(q)
        self._router._flow_table.add(packet, None, set([ws]))
        ws.send(packet)

    def _monitor_interface(self, interface):
        print("MONITOR interface", interface)
        node = interface.node
        # XXX only monitor in the topology group
        if node.get_type() != 'lxccontainer':
            print("MONITOR  -> Ignored: not in container")
            return

        # Only monitor vpp interfaces on vpp node
        if hasattr(node, 'vpp') and node.vpp is not None:
            print("MONITOR  -> Ignored: non-vpp interface on vpp node")
            return

        # We only monitor interfaces to provide data for wired channels
        channel = interface.channel
        if channel is None:
            print("MONITOR  -> Ignored: no channel")
            return
        if channel.has_type('emulatedchannel'):
            print("MONITOR  -> Ignored: belong to wireless channel")
            return

        # Don't monitor multiple interfaces per channel
        if channel in self._monitored_channels:
            print("MONITOR  -> Ignored: channel already monitored")
            return
        self._monitored_channels.add(channel)

        ip = str(node.management_interface.ip4_address)
        if not ip:
            log.error('IP of monitored Node {} is None'.format(resource.node))
            import os; os._exit(1)

        # Reuse existing websockets
        ws = self._map_ip_interface.get(ip)
        if not ws:
            ws = self._router.add_interface('websocketclient', address=ip,
                    hook=self._on_netmon_record)
            self._map_ip_interface[ip] = ws

        q_str = Q_SUB_IF.format(interface.device_name)
        print("MONITOR -> query= {}".format(q_str))
        q = self.parse_query(q_str)
        packet = Packet.from_query(q)
        self._router._flow_table.add(packet, None, set([ws]))
        ws.send(packet)

    def _monitor_emulator(self, resource):
        ns = resource
        # XXX UGLY, we have no management interface
        ip = ns.node.hostname # str(ns.node.interfaces[0].ip4_address)

        ws_ns = self._router.add_interface('websocketclient', address = ip,
                port = ns.control_port,
                hook = self._on_ns_record)
        ws = self._router.add_interface('websocketclient', address = ip,
                hook = self._on_netmon_channel_record)

        for station in ns.stations:
            if not station.managed:
                interface = [i for i in station.interfaces if i.channel == ns]
                assert len(interface) == 1
                interface = interface[0]
                identifier = interface.name
            else:
                iface = ns._sta_ifs[station]
                identifier = iface._state.uuid._uuid

            # Monitor the wireless channel for position and link rate
            q_str = Q_SUB_EMULATOR_IF.format(identifier)
            q = self.parse_query(q_str)
            packet = Packet.from_query(q)
            self._router._flow_table.add(packet, None, set([ws_ns]))
            ws_ns.send(packet)

            # We also need to subscribe on the node for the tap interfaces
            # for individual bandwidth monitoring
            tap = ns._sta_taps[station]
            q_str = Q_SUB_EMULATOR.format(tap.device_name)
            q = self.parse_query(q_str)
            packet = Packet.from_query(q)
            self._router._flow_table.add(packet, None, set([ws]))
            ws.send(packet)

    def _monitor(self, resource):
        if resource.get_type() == 'centralip':
            for uuid in self._pending_monitoring:
                pending = self.by_uuid(uuid)
                self._monitor(pending)
            self._pending_monitoring.clear()
            return

        uuid = resource.get_uuid()

        central_ip = self.by_type_str('centralip')
        if central_ip:
            central_ip = central_ip[0]

            if central_ip._state.state != ResourceState.CLEAN:
                self._pending_monitoring.add(uuid)
                return

        if uuid in self._monitored:
            return
        self._monitored.add(uuid)

#        if resource.get_type() == 'netmon':
#            if resource.node.get_type() != 'lxccontainer':
#                return
#            self._monitor_netmon(resource)

        if resource.get_type() == 'qtplayer':
            self._monitor_qtplayer(resource)

        elif resource.has_type('emulatedchannel'):
            self._monitor_emulator(resource)

        elif resource.has_type('interface'):
            self._monitor_interface(resource)

        elif resource.has_type('vppinterface'):
            self._monitor_vpp_interface(resource)

    async def __set_resource_state(self, resource, state):
        """Sets the resource state (no-lock version)

        It is important to centralize state change since some states are
        associated with Events().
        """
        prev_state = resource._state.state
        resource._state.state = state
        if state == ResourceState.CLEAN:
            # Monitoring hook
            self._monitor(resource)
            resource._state.clean.set()
            if prev_state != ResourceState.CLEAN:
                self._num_clean += 1
            log.info("Resource {} is marked as CLEAN ({}/{})".format(
                    resource.get_uuid(), self._num_clean, self._num))
        else:
            resource._state.clean.clear()
            if prev_state == ResourceState.CLEAN:
                self._num_clean -= 1
        if state == ResourceState.INITIALIZED:
            resource._state.init.set()

    async def _set_resource_state(self, resource, state):
        """Sets the resource state (lock version)
        """
        with await resource._state.lock:
            await self.__set_resource_state(resource, state)

    def _trigger_state_change(self, resource, fut):
        try:

            ret = fut.result()
            resource._state.change_success = True
            resource._state.change_value  = ret
        except ResourceNotFound as e:
            resource._state.change_success = False
            resource._state.change_value  = e
        except Exception as e:
            resource._state.change_success = False
            resource._state.change_value  = e
        resource._state.change_event.set()

    def _process_attr_dict(self, resource, attribute, attrs):
        if not isinstance(attrs, dict):
            if attribute is None:
                return False
            attrs = {attribute.name: attrs}
        resource.set_many(attrs, current=True)
        return True

    async def _task_resource_update(self, resource):
        # Monitor all FSM one by one and inform about errors.
        futs = list()
        attrs = list()
        for attr in resource.iter_attributes():
            if resource.is_local_attribute(attr.name):
                continue
            if resource.has_key_attribute(attr):
                # Those attributes are already done
                continue

            attrs.append(attr)
            fut = self.attribute_process(resource, attr)
            futs.append(fut)

        if not futs:
            self.log(resource, 'No attribute to update')
            return None

        await asyncio.gather(*futs)

        # Inform the resource about the outcome of the update process
        # Error if at least one attribute failed.
        resource._state.change_success = all(
                resource._state.attr_change_success[attr.name]
                for attr in attrs)
        self.log(resource,
                'All attributes FSM terminated with success={}'.format(
                    resource._state.change_success))

        if resource._state.change_success:
            ret = [ resource._state.attr_change_value[attr.name]
                    for attr in attrs]
            return ret
        else:
            raise NotImplementedError('At least one attribute failed')

    async def _task_resource_keys(self, resource):
        # Monitor all FSM one by one and inform about errors.
        futs = list()
        attrs = list()

        for key in resource.get_keys():
            for attr in key:
                if resource.is_local_attribute(attr.name):
                    continue
                attrs.append(attr)
                fut = self.attribute_process(resource, attr)
                futs.append(fut)

        if not futs:
            self.log(resource, 'No key attribute to update')
            return None

        await asyncio.gather(*futs)

        # Inform the resource about the outcome of the update process
        # Error if at least one attribute failed.
        resource._state.change_success = all(
                resource._state.attr_change_success[attr.name]
                for attr in attrs)
        self.log(resource,
                'KEY attributes FSM terminated with success={}'.format(
                    resource._state.change_success))

        if resource._state.change_success:
            ret = resource._state.attr_change_value
            return ret
        else:
            raise NotImplementedError('At least one attribute failed')

    #--------------------------------------------------------------------------
    # Logging
    #--------------------------------------------------------------------------

    def log(self, resource, msg=None):
        resource._state.log.append(msg)

        # Display on screen
        #pfx = '[{}] {}: '.format(resource.get_type(), resource.get_uuid())
        #print(pfx, msg)

    def attr_log(self, resource, attribute, msg):
        resource._state.attr_log[attribute.name].append(msg)

        # Display on screen
        #pfx = '[{}] {} / {}: '.format(resource.get_type(), resource.get_uuid(),
        #        attribute.name)
        #print(pfx, msg)

    #--------------------------------------------------------------------------

    async def _process_resource(self, resource):
        """
        We need to schedule the first set of subresources, knowing others will
        be orchestrated by the operators
         - subresources need to enter the system in order
           -> we just add them to the manager in time
         - but they need to be managed by the system
         - in particular, the owner waits for the system to complete
         subresoruces: this is the implementation of __get__ __create__
         __delete__ in the base resource
        """
        pfx = '[{}] {}: '.format(resource.get_type(), resource.get_uuid())

        self.log(resource, 'Starting FSM...')

        # When a resource is managed, it will get automatically monitored by
        # adding the netmon resource on it.
        from vicn.resource.node import Node
        if resource.get_type() == 'lxccontainer':
            self.log(resource,
                    'Associating monitoring to lxc container resource...')
            instance = self.create('netmon', node=resource)
            self.commit_resource(instance)

        # FIXME
        elif resource.get_type() == 'physical' and resource.managed and \
                    len(self.by_type_str('emulatedchannel')) > 0:
            self.log(resource,
                    'Associating monitoring to physical node resource...')
            instance = self.create('netmon', node=resource)
            self.commit_resource(instance)

        state = None

        while True:
            with await resource._state.lock:

                # FSM implementation
                state = resource._state.state
                self.log(resource, 'Current state is {}'.format(state))

                if state == ResourceState.ERROR:
                    e = resource._state.change_value
                    print("------")
                    import traceback; traceback.print_tb(e.__traceback__)
                    log.error('Resource: {} - Exception: {}'.format(pfx, e))
                    return
                    import os; os._exit(1)
                elif state == ResourceState.UNINITIALIZED:
                    pending_state = ResourceState.PENDING_DEPS
                elif state == ResourceState.DEPS_OK:
                    pending_state = ResourceState.PENDING_INIT

                elif state == ResourceState.INITIALIZED:
                    pending_state = ResourceState.PENDING_GET

                elif state == ResourceState.GET_DONE:
                    if resource.get_keys():
                        pending_state = ResourceState.PENDING_KEYS
                    else:
                        pending_state = ResourceState.PENDING_CREATE

                elif state == ResourceState.KEYS_OK:
                    pending_state = ResourceState.PENDING_CREATE

                elif state in [ResourceState.CREATED, ResourceState.DIRTY]:
                    pending_state = ResourceState.PENDING_UPDATE

                elif state == ResourceState.DELETED:
                    raise NotImplementedError
                    # Nothing to do unless explicitely requested
                    pending_state = None

                elif state in [
                    ResourceState.PENDING_DEPS,
                    ResourceState.PENDING_INIT,
                    ResourceState.PENDING_CREATE,
                    ResourceState.PENDING_DELETE,
                    ResourceState.CLEAN
                ]:
                    # Nothing to do
                    pending_state = None
                else:
                    raise RuntimeError

            # Implement state changes
            #
            # If a task is already pending, we simply wait for it to complete
            if pending_state is None:
                # Wait for an external change
                self.log(resource, 'Nothing to do. Waiting for event...')
                await resource._state.change_event.wait()
                self.log(resource, 'Wake up from event')
                resource._state.change_event.clear()
                continue

            if pending_state == ResourceState.PENDING_DEPS:
                # XXX Maybe for any action, we need to wait for dependencies to
                # be up to date
                task = async_task(functools.partial(self._resource_wait_dependencies, resource))()

            elif pending_state == ResourceState.PENDING_INIT:
                task = self._task_resource_action(resource, '__initialize__')

            elif pending_state == ResourceState.PENDING_GET:
                task = self._task_resource_action(resource, '__get__')
                if isinstance(task, BashTask):
                    task.set_default_parse_for_get()

            elif pending_state == ResourceState.PENDING_KEYS:
                task = async_task(functools.partial(self._task_resource_keys, resource))()

            elif pending_state == ResourceState.PENDING_CREATE:
                task = self._task_resource_action(resource, '__create__')

            elif pending_state == ResourceState.PENDING_UPDATE:
                # Instead of tasks, we wait for many smaller autoamtons to
                # terminate
                await resource._state.write_lock.acquire()
                task = async_task(functools.partial(self._task_resource_update, resource))()

            elif pending_state == ResourceState.PENDING_DELETE:
                task = self._task_resource_action(resource, '__delete__')

            else:
                raise RuntimeError

            if task is not None and not isinstance(task, EmptyTask):
                resource._state.change_success = None # undetermined state
                state_change = functools.partial(self._trigger_state_change, resource)
                task.add_done_callback(state_change)
                self.schedule(task, resource)

                self.log(resource, 'Trigger {} -> {}. Waiting task completion'.format(
                            state, pending_state))
                await resource._state.change_event.wait()
                resource._state.change_event.clear()
                self.log(resource, 'Completed {} -> {}. Success = {}'.format(
                            state, pending_state, resource._state.change_success))

            # If no task, can assume there is an instant switch to the next step...

            # Update state based on task results
            if pending_state == ResourceState.PENDING_DEPS:
                if resource._state.change_success == True:
                    self.log(resource, 'DEPS done.')
                    new_state = ResourceState.DEPS_OK
                else:
                    e = resource._state.change_value
                    log.error('Cannot wait resource dependencies {} : {}'.format(
                            resource.get_uuid(), e))
                    new_state = ResourceState.ERROR

            elif pending_state == ResourceState.PENDING_INIT:
                if resource._state.change_success == True:
                    attrs = resource._state.change_value
                    self.log(resource, 'INIT done.')
                    new_state = ResourceState.INITIALIZED
                else:
                    e = resource._state.change_value
                    import traceback; traceback.print_tb(e.__traceback__)
                    log.error('Cannot setup resource {} : {}'.format(
                            resource.get_uuid(), e))
                    import os; os._exit(1)

            elif pending_state == ResourceState.PENDING_GET:
                if resource._state.change_success == True:
                    attrs = resource._state.change_value
                    if not isinstance(attrs, ReturnValue):
                        self.log(resource, S_INIT_DONE.format(attrs))
                        self._process_attr_dict(resource, None, attrs)
                    new_state = ResourceState.CREATED
                else:
                    e = resource._state.change_value
                    if ENABLE_LXD_WORKAROUND and                        \
                            resource.get_type() != 'lxccontainer' and   \
                            isinstance(e, LxdNotFound):
                        # "not found" is the normal exception when the container
                        # does not exists. anyways the bug should only occur
                        # with container.execute(), not container.get()
                        log.warning('LXD Fix (not found). Reset resource')
                        new_state = ResourceState.INITIALIZED
                        resource._state.change_success = True
                    elif ENABLE_LXD_WORKAROUND and isinstance(e, LXDAPIException):
                        # "not found" is the normal exception when the container
                        # does not exists. anyways the bug should only occur
                        # with container.execute(), not container.get()
                        log.warning('LXD Fix (API error). Reset resource')
                        new_state = ResourceState.INITIALIZED
                        resource._state.change_success = True
                    elif isinstance(e, ResourceNotFound):
                        # The resource does not exist
                        self.log(resource, S_GET_DONE.format(
                                    resource._state.change_value))
                        new_state = ResourceState.GET_DONE
                        resource._state.change_value = None
                        resource._state.change_success = True
                    else:
                        e = resource._state.change_value
                        log.error('Cannot get resource state {} : {}'.format(
                                    resource.get_uuid(), e))
                        new_state = ResourceState.ERROR

            elif pending_state == ResourceState.PENDING_KEYS:
                if resource._state.change_success == True:
                    new_state = ResourceState.KEYS_OK
                    self.log(resource, S_KEYS_OK)
                else:
                    e = resource._state.change_value
                    self.log(resource, 'KEYS failed: {}'.format(e))

                    if ENABLE_LXD_WORKAROUND and isinstance(e, LxdNotFound):
                        log.warning('LXD Fix (not found). Reset resource')
                        new_state = ResourceState.CREATED
                        resource._state.change_success = True
                    else:
                        e = resource._state.change_value
                        log.error('Cannot create resource {} : {}'.format(
                                resource.get_uuid(), e))

            elif pending_state == ResourceState.PENDING_CREATE:
                if resource._state.change_success == True:
                    attrs = resource._state.change_value
                    if not isinstance(attrs, ReturnValue):
                        self.log(resource, S_CREATE_OK.format(attrs))
                        self._process_attr_dict(resource, None, attrs)
                    new_state = ResourceState.CREATED
                else:
                    e = resource._state.change_value

                    if ENABLE_LXD_WORKAROUND and isinstance(e, LxdNotFound):
                        log.warning('LXD Fix (not found). Reset resource')
                        new_state = ResourceState.INITIALIZED
                        resource._state.change_success = True
                    elif ENABLE_LXD_WORKAROUND and \
                            isinstance(e, LXDAPIException):
                        log.warning('LXD Fix (API error). Reset resource')
                        new_state = ResourceState.INITIALIZED
                        resource._state.change_success = True
                    elif 'File exists' in str(e):
                        new_state = ResourceState.CREATED
                        resource._state.change_success = True
                    elif 'dpkg --configure -a' in str(e):
                        resource._dpkg_configure_a = True
                        new_state = ResourceState.INITIALIZED
                        resource._state.change_success = True
                    else:
                        self.log(resource, 'CREATE failed: {}'.format(e))
                        new_state = ResourceState.ERROR

            elif pending_state == ResourceState.PENDING_UPDATE:
                if resource._state.change_success == True:
                    self.log(resource, 'Update finished, resource is CLEAN.')
                    new_state = ResourceState.CLEAN
                    resource._state.write_lock.release()
                else:
                    e = resource._state.change_value
                    self.log(resource, 'UPDATE failed: {}'.format(e))

                    if ENABLE_LXD_WORKAROUND and isinstance(e, LxdNotFound):
                        log.warning('LXD Fix (not found). Reset resource')
                        new_state = ResourceState.CREATED
                        resource._state.change_success = True
                        resource._state.write_lock.release()
                    else:
                        resource._state.write_lock.release()
                        new_state = ResourceState.ERROR

            elif pending_state == ResourceState.PENDING_DELETE:
                raise NotImplementedError
                new_state = None
            else:
                raise RuntimeError

            await self._set_resource_state(resource, new_state)
