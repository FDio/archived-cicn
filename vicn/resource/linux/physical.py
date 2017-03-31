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

import os
import stat
import logging
import subprocess
import shlex

from netmodel.model.type            import String, Integer
from netmodel.util.misc             import is_local_host
from netmodel.util.socket           import check_port
from vicn.core.attribute            import Attribute
from vicn.core.commands             import Command, ReturnValue
from vicn.core.exception            import ResourceNotFound, VICNException
from vicn.core.task                 import Task, task, EmptyTask
from vicn.resource.linux.keypair    import Keypair
from vicn.resource.node             import Node, DEFAULT_USERNAME
from vicn.resource.node             import DEFAULT_SSH_PUBLIC_KEY
from vicn.resource.node             import DEFAULT_SSH_PRIVATE_KEY

log = logging.getLogger(__name__)

CMD_SSH_COPY_ID = 'ssh-copy-id {ssh_options} -i {public_key} -p {port} ' \
                  '{user}@{host}'
CMD_SSH = 'ssh {ssh_options} -i {private_key} -p {port} ' \
          '{user}@{host} {command}'
CMD_SSH_NF = 'ssh -n -f {ssh_options} -i {private_key} -p {port} ' \
             '{user}@{host} {command}'

FN_KEY = os.path.expanduser(os.path.join(
            '~', '.vicn', 'ssh_client_cert', 'ssh_client_key'))

class Physical(Node):
    """
    Resource: Physical

    Physical node
    """

    hostname = Attribute(String, description = 'Hostname or IP address',
            mandatory = True)
    ssh_port = Attribute(Integer, description = 'SSH port number',
            default = 22)

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.node_with_kernel = self

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __subresources__(self):
        """
        Require a SSH keypair to be present for authentication on nodes
        """
        return Keypair(node = None, key = FN_KEY)

    def __initialize__(self):
        if not is_local_host(self.hostname):
            """
            Initialization require the ssh port to be open on the node, and the ssh
            public key to be copied on the remote node.
            """
            if not check_port(self.hostname, self.ssh_port):
                raise VICNException

            tasks = list()
            modes = (True, False) if DEFAULT_USERNAME != 'root' else (True,)
            for as_root in modes:
                tasks.append(self._setup_ssh_key(as_root))
            return Task.__concurrent__(*tasks)
        else:
            return EmptyTask()

    __delete__ = None

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    @task
    def _setup_ssh_key(self, as_root):
        os.chmod(DEFAULT_SSH_PUBLIC_KEY, stat.S_IRUSR | stat.S_IWUSR)
        os.chmod(DEFAULT_SSH_PRIVATE_KEY, stat.S_IRUSR | stat.S_IWUSR)
        cmd_params = {
            'public_key'    : DEFAULT_SSH_PUBLIC_KEY,
            'ssh_options'   : '',
            'port'          : self.ssh_port,
            'user'          : 'root' if as_root else DEFAULT_USERNAME,
            'host'          : self.hostname,
        }

        c = Command(CMD_SSH_COPY_ID, parameters = cmd_params)

        return self._do_execute_process(c.full_commandline, output=False)

    #--------------------------------------------------------------------------
    # Public API
    #--------------------------------------------------------------------------

    def _do_execute_process(self, command, output = False):
        p = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE)
        if output:
            out, err = p.communicate()
            return ReturnValue(p.returncode, stdout=out, stderr=err)

        p.wait()
        return ReturnValue(p.returncode)

    def _do_execute_ssh(self, command, output=False, as_root=False,
            ssh_options=None):
        if not ssh_options:
            ssh_options = dict()
        cmd_params = {
            'private_key'   : DEFAULT_SSH_PRIVATE_KEY,
            'ssh_options'   : ' '.join(['-o {}={}'.format(k, v)
                    for k, v in ssh_options.items()]),
            'port'          : self.ssh_port,
            'user'          : 'root' if as_root else DEFAULT_USERNAME,
            'host'          : self.hostname,
            'command'       : shlex.quote(command),
        }

        if (command[-1] != '&'):
            c = Command(CMD_SSH, parameters = cmd_params)
        else:
            c = Command(CMD_SSH_NF, parameters = cmd_params)

        return self._do_execute_process(c.full_commandline_nobashize, output)

    def execute(self, command, output=False, as_root=False):
        if is_local_host(self.hostname):
            rv = self._do_execute_process(command, output = output)
        else:
            rv = self._do_execute_ssh(command, output = output,
                    as_root = as_root)
        return rv
