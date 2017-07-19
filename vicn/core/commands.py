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

import inspect
import logging
import shlex

from vicn.core.exception import CommandException

log = logging.getLogger(__name__)

#------------------------------------------------------------------------------
# Helper functions
#------------------------------------------------------------------------------

def bashize(command):
    ret = "bash -c " + shlex.quote(command) 
    return ret

def parenthesize(command):
    return '({})'.format(command)

def do_parenthesize(command):
    if '&&' in command or '||' in command:
        if command[0] == '(':
            return command
        else:
            return parenthesize(command)
    else:
        return command

#------------------------------------------------------------------------------

class ReturnValue:
    def __init__(self, return_value = None, stdout = None, stderr = None):
        self._return_value = return_value

        # We use accessors since it seems impossible to trigger properties from
        # __init__
        self._set_stdout(stdout)
        self._set_stderr(stderr)

    def __repr__(self):
        return '<ReturnValue ({}) - OUT [{}] - ERR [{}]>'.format(
                self._return_value, self._stdout, self._stderr)

    def __str__(self):
        return self.__repr__()

    def _clean(self, value):
        if value is None or isinstance(value, str):
            return value
        return value.decode('utf-8').strip()

    def _set_stdout(self, value):
        self._stdout = self._clean(value)

    def _set_stderr(self, value):
        self._stderr = self._clean(value)

    @property
    def stdout(self):
        return self._stdout

    @stdout.setter
    def stdout(self, value):
        return self._set_stdout(value)

    @property
    def stderr(self):
        return self._stderr

    @stderr.setter
    def stderr(self, value):
        return self._set_stderr(value)

    @property
    def return_value(self):
        return self._return_value

    @return_value.setter
    def return_value(self, value):
        self._return_value = value

    def __bool__(self):
        return self._return_value == 0

#------------------------------------------------------------------------------

class Command:
    """
    Bash command

    Todo:
      - Commands with ; should be "bashized"
    """
    def __init__(self, commandline, node=None, parameters = None, 
            description = None, callback = None, blocking=True, lock=None):
        self._commandline = commandline
        self._node = node
        self._parameters = parameters if parameters else dict()
        self._description = description
        self._callback = callback
        self._blocking = blocking
        self._lock = lock

    def __str__(self):
        try:
            return self.full_commandline
        except:
            return self.commandline + ' -- ' + str(self.parameters)

    def __repr__(self):
        return '<Command {} -- {}'.format(self.commandline, self.parameters)

    @property
    def commandline(self):
        return self._commandline

    @property
    def full_commandline(self):
        cmd = self._commandline.format(**self._parameters)
        if ('||' in cmd or '&&' in cmd or '|' in cmd or '<' in cmd or 
                '>' in cmd):
            return bashize(cmd)
        return cmd

    @property
    def full_commandline_nobashize(self):
        """
        TMP to fix issue with bashize heuristic above...
        """
        cmd = self._commandline.format(**self.parameters)
        return cmd

    @property
    def command(self):
        return self

    @property
    def node(self):
        return self._node

    @node.setter
    def node(self, node):
        self._node = node

    @property
    def parameters(self):
        return self._parameters

    @parameters.setter
    def parameters(self, parameters):
        self._parameters = parameters

    @property
    def description(self):
        if not self._description:
            return self._description
        return self._description.format(**self.parameters)

    @property
    def success_callback(self):
        return self._on_success

    @property
    def failure_callback(self):
        return self._on_failure

    @property
    def blocking(self):
        return self._blocking

    @property
    def lock(self):
        return self._lock

    def apply(self, params):
        self._parameters.update(params)
        return self

    def __and__(self, other):
        commandline = self.commandline + ' && ' + other.commandline
        all_params = dict(i for c in (self, other) 
                for i in c.parameters.items())
        return Command(commandline, parameters = all_params)

    def __or__(self, other):
        commandline = self.commandline + ' || ' + other.commandline
        all_params = dict(i for c in (self, other) 
                for i in c.parameters.items())
        return Command(commandline, parameters = all_params)

    def __bool__(self):
        return bool(self._commandline)

    def submit(self):
        CMD_MGR.execute([self])

    def execute(self):
        cmd = self.full_commandline
        cmd_str = cmd[:77]+'...' if len(cmd) > 80 else cmd
        log.debug('Node {}: {} ({})'.format(self.node.name, cmd_str, 
                    self._description))

        rv = self.node.execute(cmd)

        if not rv:
            raise CommandException
        if self._callback:
            if self._lock:
                self._lock.acquire()
            self._callback(rv)
            if self._lock:
                self._lock.release()
        return rv

#------------------------------------------------------------------------------

class BackgroundCommand(Command):
    pass

#------------------------------------------------------------------------------

class Commands(Command):

    def __init__(self):
        self._commands = list()
        self._node = None

    def __repr__(self):
        return '<Commands {}>'.format(str(self))

    def __str__(self):
        return self.commandline

    def add(self, command):
        if not command:
            return
        if not isinstance(command, Command):
            command = Command(command)
        self._commands.append(command)

    def _do_command(self, sep):
        if len(self.commands) == 1:
            full_cmd = sep.join(c.commandline for c in self.commands)
        else:
            full_cmd = sep.join(do_parenthesize(c.commandline) 
                    for c in self.commands)
        all_params = dict(i for c in self.commands 
                for i in c.parameters.items())

        return Command(full_cmd, parameters = all_params)

    @property
    def command(self):
        raise NotImplementedError('Not implemented')

    @property
    def commandline(self):
        return self.command.commandline

    @property
    def parameters(self):
        parameters = dict()
        for command in self.commands:
            parameters.update(command.parameters)
        return parameters

    @parameters.setter
    def parameters(self, parameters):
        for command in self.commands:
            command.parameters = parameters

    @property
    def commands(self):
        return self._commands

    def apply(self, params):
        self._commands = [c.apply(params) for c in self._commands]
        return self._commands

    __lshift__ = add

    def __bool__(self):
        return any(bool(c) for c in self._commands)

#------------------------------------------------------------------------------

class ParallelCommands(Commands):
    @property
    def command(self):
        log.warning('Commands executed sequentially')
        return self._do_command(';')

#------------------------------------------------------------------------------

class SequentialCommands(Commands):
    @property
    def command(self, fatal = True):
        SEP = ' && ' if fatal else '; '
        return self._do_command(SEP)

#------------------------------------------------------------------------------

def sequential_bash_from_docstring(fn):
    def decorator(*args, **kwargs):
        c = SequentialCommands()

        desc = None
        for line in fn.__doc__.splitlines():
            line = line.strip()
            if not line:
                continue
            if line.startswith('#'):
                desc = line[1:].strip()
                continue
            c << Command(line, description = desc)
            desc = None

        # XXX we don't support keyword args
        arg_info = inspect.getargspec(fn)
        assert not arg_info.varargs
        c.parameters = dict(zip(arg_info.args, args))
        log.debug('sequential_bash_from_docstring: {}'.format(c))

        # Execute the code in the function
        fn(*args, **kwargs)

        return c

    return decorator

bash_from_docstring = sequential_bash_from_docstring

def execute_on_node(fn):
    """
    Decorator: execute the command returned by the function on the node found
    in attributes. Note that such an attribute should be available.
    This assumes the function returns a command

    We need output in case apply_rx is used. This should be made an option
    """

    def wrapper(self, *args, **kwargs):
        return self.node.execute(fn(self, *args, **kwargs), output = True)
    return wrapper

def apply_rx(rx):
    """
    Apply a compiled regular expression to the result of the decorated function.
    Returns a dict (whose keys should be attributes of the resource that need
    to be updated).
    """

    def decorator(fn):
        def wrapper(*args, **kwargs):
            ret = fn(*args, **kwargs)
            return [m.groupdict() for m in rx.finditer(ret.stdout)]
        return wrapper
    return decorator
