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
import concurrent.futures
import functools
import logging
import shlex
import subprocess
import os

from vicn.core.scheduling_algebra   import SchedulingAlgebra
from vicn.core.commands             import ReturnValue
from vicn.core.exception            import ResourceNotFound
from vicn.core.commands             import Command, SequentialCommands
from netmodel.util.process          import execute_local

log = logging.getLogger(__name__)

EXECUTOR=concurrent.futures.ThreadPoolExecutor
#EXECUTOR=concurrent.futures.ProcessPoolExecutor

#Sets the number of task workers to the number of CPU threads+1
MAX_WORKERS = os.cpu_count()+1

class BaseTask:
    """Base class for all tasks
    """

    def __init__(self):
        self._future = asyncio.Future()

    def terminate(self):
        pass

    def start(self):
        pass

    def stop(self):
        pass

    def get_future(self):
        return self._future

    def add_done_callback(self, cb):
        self._future.add_done_callback(cb)

    def __repr__(self):
        return '<BaseTask>'

class ConcurrentMixin:
    async def execute(self):
        try:
            for t in self._elements:
                await t.execute()
            rets = await asyncio.gather(*[t.get_future()
                    for t in self._elements])

            # The result value is the "union" of all result values
            # In case of tasks setting the same attributes, they are merged
            # into a list

            dic = dict()
            for ret in rets:
                # Ideally we should get all attribute names, and properly
                # insert Nones. So far we assume all dicts are present and
                # complete.
                if not isinstance(ret, dict):
                    continue
                for k, v in ret.items():
                    if k in dic:
                        if not isinstance(dic[k], list):
                            dic[k] = [dic[k]]
                        dic[k].append(v)
                    else:
                        dic[k] = [v]
            self.get_future().set_result(dic)
        except Exception as e:
            self.get_future().set_exception(e)

class SequentialMixin:
    async def execute(self):
        try:
            for t in self._elements:
                await t.execute()
                await t.get_future()
            self.get_future().set_result(None)
        except Exception as e:
            self.get_future().set_exception(e)

class CompositionMixin:
    async def execute(self):
        try:
            ret = None
            for t in self._elements:
                ret = (ret,) if ret is not None else tuple()
                await t.execute(*ret)
                ret = await t.get_future()
            self.get_future().set_result(ret)
        except Exception as e:
            print('we need to cancel tasks not executed...')
            self.get_future().set_exception(e)

Task, EmptyTask = SchedulingAlgebra(BaseTask, ConcurrentMixin,
        CompositionMixin, SequentialMixin)

def task(fn):
    def decorator(*args, **kwargs):
        return PythonTask(fn, *args, **kwargs)
    return decorator

def async_task(fn, *t_args, **t_kwargs):
    def decorator(*args, **kwargs):
        all_args = tuple() + t_args + args
        all_kwargs = dict()
        all_kwargs.update(t_kwargs)
        all_kwargs.update(kwargs)
        return PythonAsyncTask(fn, *args, **kwargs)
    return decorator

def inline_task(fn):
    def decorator(*args, **kwargs):
        return PythonInlineTask(fn, *args, **kwargs)
    return decorator

async def wait_task(task):
    return await task.get_future()

async def run_task(task, manager):
    manager.schedule(task)
    ret = await wait_task(task)
    return ret

async def wait_concurrent_tasks(tasks):
    await wait_task(Task.__concurrent__(*tasks))

wait_task_task = async_task(wait_task)

def get_attribute_task(resource, attribute_name):
    @async_task
    async def func():
        return await resource.async_get(attribute_name)
    return func()

def set_attributes_task(resource, attribute_dict):
    # The difficulty is in setting the pending value without triggering the
    # manager, and executing the task by ourselves !
    raise NotImplementedError

def get_attributes_task(resource, attribute_names):
    assert len(attribute_names) == 1
    attribute_name = attribute_names[0]

    @async_task
    async def func():
        await resource._state.manager.wait_attr_init(resource, attribute_name)
        ret = await resource.async_get(attribute_name)
        return {attribute_name: ret}
    return func()

def _get_func_desc(f):
    """
    Returns a string representation of a function for logging purposes.

    Todo: args and keywords (including from partial)
    """
    partial = isinstance(f, functools.partial)
    if partial:
        f = f.func

    s = ''
    if hasattr(f, '__name__'):
        s += f.__name__
    if hasattr(f, '__doc__') and f.__doc__:
        if s:
            s += ' : '
        s += f.__doc__

    return 'partial<{}>'.format(s) if partial else s


class PythonTask(Task):
    def __init__(self, func, *args, **kwargs):
        super().__init__()
        self._func = func
        self._args = args
        self._kwargs = kwargs

    def _done_callback(self, fut):
        try:
            self._future.set_result(fut.result())
        except Exception as e:
            self._future.set_exception(e)

    async def execute(self, *args, **kwargs):
        all_args = self._args + args
        all_kwargs = dict()
        all_kwargs.update(self._kwargs)
        all_kwargs.update(kwargs)

        partial = functools.partial(self._func, *all_args, **all_kwargs)

        loop = asyncio.get_event_loop()
        fut = loop.run_in_executor(None, partial)
        fut.add_done_callback(self._done_callback)

    def __repr__(self):
        s = _get_func_desc(self._func)
        return '<Task[py] {}>'.format(s) if s else '<Task[py]>'

class PythonAsyncTask(PythonTask):
    async def execute(self, *args, **kwargs):
        all_args = self._args + args
        all_kwargs = dict()
        all_kwargs.update(self._kwargs)
        all_kwargs.update(kwargs)

        partial = asyncio.coroutine(self._func)(*all_args, **all_kwargs)

        fut = asyncio.ensure_future(partial)
        fut.add_done_callback(self._done_callback)

    def __repr__(self):
        s = _get_func_desc(self._func)
        return '<Task[apy] {}>'.format(s) if s else '<Task[apy]>'

class PythonInlineTask(PythonTask):
    async def execute(self, *args, **kwargs):
        all_args = self._args + args
        all_kwargs = dict()
        all_kwargs.update(self._kwargs)
        all_kwargs.update(kwargs)

        try:
            ret = self._func(*all_args, **all_kwargs)
            self._future.set_result(ret)
        except Exception as e:
            self._future.set_exception(e)
        return self._future

    def __repr__(self):
        s = _get_func_desc(self._func)
        return '<Task[ipy] {}>'.format(s) if s else '<Task[ipy]>'

class BashTask(Task):
    def __init__(self, node, cmd, parameters=None, parse=None, as_root=False,
            output=False, pre=None, post=None, lock=None):
        super().__init__()
        self._node = node
        self._cmd = cmd
        self._params = parameters if parameters else dict()
        self._parse = parse
        self._pre = pre
        self._post = post
        self._lock = lock

        self._output = output
        self._as_root = as_root

    def _default_parse_for_get(self, rv):
        if not bool(rv):
            raise ResourceNotFound

    def set_default_parse_for_get(self):
        if self._parse is None:
            self._parse = self._default_parse_for_get

    def _done_callback(self, fut):
        """
        Note: extends the functionality of the parent _done_callback
        """
        try:
            rv = fut.result()
            if self._parse is None and rv.return_value != 0:
                raise Exception('Bash command failed on node {}'.format(self._node.name), self.get_full_cmd(), rv)
            if self._post:
                self._post()
            if self._parse:
                rv = self._parse(rv)
            self._future.set_result(rv)
        except Exception as e:
            self._future.set_exception(e)
        if self._lock:
            self._lock.release()

    def get_full_cmd(self):
        c = SequentialCommands()
        desc = None
        for line in self._cmd.splitlines():
            line = line.strip()
            if not line:
                continue
            if line.startswith('#'):
                desc = line[1:].strip()
                continue
            c << Command(line, description = desc)
            desc = None

        c.parameters = self._params
        return c.command.full_commandline

    async def execute(self, *args, **kwargs):
        """Execute the task, enforcing any eventual locking.

        Returns:
            asyncio.Future

        Upon completion (eventually error), the Task's future is set.
        """
        if len(args) == 1:
            dic, = args
            if isinstance(dic, dict):
                self._params.update(dic)
        if self._pre:
            self._pre()

        func = self._node.execute if self._node else execute_local
        # It is important that the command is contructed only just before it is
        # executed, so that any object passed as parameters is deferenced right
        # on time.
        cmd = self.get_full_cmd()
        partial = functools.partial(func, cmd, output = bool(self._parse))

        node_str = self._node.name if self._node else '(LOCAL)'
        cmd_str = cmd[:77] + '...' if len(cmd) > 80 else cmd
        log.info('Execute: {} - {}'.format(node_str, cmd_str))

        if self._lock:
            # We need to do lock/unlock around the task execution
            # Locking now will early block other tasks, but will at the same
            # time delay them entering the executor queue; so this is
            # equivalent
            await self._lock.acquire()

        loop = asyncio.get_event_loop()
        fut = loop.run_in_executor(None, partial)
        fut.add_done_callback(self._done_callback)

    def execute_blocking(self):
        rv = self._node.execute(self.get_full_cmd(), output=True)
        if self._parse:
            rv = self._parse(rv)
        return rv

    def __repr__(self):
        return '<Task[bash] {} / {}>'.format(self._cmd, self._params)

class TaskManager:
    def __init__(self):
        executor = EXECUTOR() if MAX_WORKERS is None \
                else EXECUTOR(max_workers=MAX_WORKERS)
        loop = asyncio.get_event_loop()
        loop.set_default_executor(executor)

    def schedule(self, task, resource = None):
        """All instances of BaseTask can be scheduled

        Here we might decide to do more advanced scheduling, like merging bash
        tasks, etc. thanks to the task algebra.
        """
        uuid = resource.get_uuid() if resource else '(unknown)'
        log.info('Scheduling task {} for resource {}'.format(task, uuid))
        asyncio.ensure_future(task.execute())

@task
def ParseRegexTask(rv):
    return [m.groupdict() for m in rx.finditer(rv.stdout)]
