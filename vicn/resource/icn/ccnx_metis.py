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

from functools import wraps

from netmodel.model.type                import String, Integer, Bool
from vicn.core.attribute                import Attribute
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource_mgr             import wait_resource_task
from vicn.core.task                     import BashTask, EmptyTask, task
from vicn.resource.icn.ccnx_keystore    import MetisKeystore
from vicn.resource.icn.face             import L2Face, L4Face, FaceProtocol
from vicn.resource.icn.face             import DEFAULT_ETHER_PROTO
from vicn.resource.icn.forwarder        import Forwarder
from vicn.resource.linux.file           import TextFile
from vicn.resource.linux.service        import Service

METIS_CONTROL_BASELINE = (
    'metis_control --keystore {keystore_file} --password {password}')

CMD_ADD_LISTENER_ETHER = (
        'add listener ether ether{conn_id} {listener.src_nic.device_name} '
        '{listener.ether_proto}')
CMD_ADD_LISTENER_L4 = 'add listener {protocol} transport{conn_id} {infos}'
CMD_ADD_CONNECTION_ETHER = ('add connection ether {face.id} {face.dst_mac} '
        '{face.src_nic.device_name}')
CMD_ADD_CONNECTION_L4 = ('add connection {protocol} {face.id} {face.dst_ip} '
        '{face.dst_port} {face.src_ip} {face.src_port}')
CMD_ADD_ROUTE = 'add route {route.face.id} ccnx:{route.prefix} {route.cost}'
METIS_DAEMON_BOOTSTRAP = (
    'metis_daemon --port {port} --daemon --log-file {log_file} '
    '--capacity {cs_size} --config {config}')
METIS_DAEMON_STOP = "killall -9 metis_daemon"

BASE_CONN_NAME = "conn"

METIS_DEFAULT_PORT = 9596

METIS_ETC_DEFAULT = "/etc/default/metis-forwarder"

#------------------------------------------------------------------------------
# Listeners
#------------------------------------------------------------------------------

class MetisListener:

    def __init__(self, protocol):
        self.protocol = protocol

    @staticmethod
    def listener_from_face(face):
        if face.protocol is FaceProtocol.ether:
            return MetisEtherListener(face.protocol, face.src_nic, 
                    face.ether_proto)
        elif face.protocol in [FaceProtocol.tcp4, FaceProtocol.tcp6, 
        FaceProtocol.udp4, FaceProtocol.udp6]:
            return MetisL4Listener(face.protocol, face.src_ip, face.src_port)
        else:
            raise ValueError("Metis only supports Ethernet and TCP/UDP faces")

class MetisEtherListener(MetisListener):

    def __init__(self, protocol, src_nic, ether_proto=DEFAULT_ETHER_PROTO):
        super().__init__(protocol)
        self.src_nic = src_nic
        self.ether_proto = ether_proto

    def get_setup_command(self, conn_id):
        return CMD_ADD_LISTENER_ETHER.format(listener = self, 
                conn_id = conn_id)

    def __eq__(self, other):
        return (isinstance(other, MetisEtherListener)
                and (other.src_nic == self.src_nic)
                and (other.ether_proto == self.ether_proto))

    def __ne__(self, other):
        return ((not isinstance(other, MetisEtherListener))
                or (other.src_nic != self.src_nic)
                or (other.ether_proto != self.ether_proto))

class MetisL4Listener(MetisListener):

    def __init__(self, protocol, src_ip, src_port):
        super().__init__(protocol)
        self.src_ip = src_ip
        self.src_port = src_port

    def _get_proto_as_str(self):
        if self.protocol in (FaceProtocol.tcp4, FaceProtocol.tcp6):
            return "tcp"
        elif self.protocol in (FaceProtocol.udp4, FaceProtocol.udp6):
            return "udp"

    def get_setup_command(self, conn_id):
        infos = '{} {}'.format(self.src_ip, self.src_port)

        return CMD_ADD_LISTENER_L4.format(protocol = self._get_proto_as_str(),
                conn_id = conn_id, infos = infos)

    def __eq__(self, other):
        return (isinstance(other, MetisL4Listener) and
                self.protocol == other.protocol and
                self.src_ip == other.src_ip and
                self.src_port == other.src_port)

#------------------------------------------------------------------------------

class MetisForwarder(Forwarder, Service):
    __capabilities__ = set(['ICN_SUITE_CCNX_1_0'])
    __package_names__ = ['metis-forwarder']
    __service_name__ = "metis-forwarder"

    log_file = Attribute(String, description = 'File for metis logging', 
            default = '/tmp/ccnx-metis.log') # '/dev/null')
    port = Attribute(Integer, description = 'TCP port for metis', 
            default = 9695)
    gen_config = Attribute(Bool, 
            description = 'Set to record all metis commands in a config file',
            default = True)
    config_file = Attribute(String, default = '/root/.ccnx_metis.conf') 

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._nb_conn = 0
        self._listeners = []
        self._listeners_idx = 0
        self.keystore = None

        # Cache
        self._faces = set()
        self._routes = set()

        # Internal subresources
        self._config = None

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        return ('CentralICN',)

    def __subresources__(self):
        self.keystore = MetisKeystore(node = self.node, owner = self)
        self.env_file = self._write_environment_file()
        return self.keystore | self.env_file

    @task
    def __get__(self):
        raise ResourceNotFound

    def __create__(self):

        # Alternatively, we might put all commands in a configuration file
        # before starting the forwarder. In that case, we need to restart it if
        # it is already started.
        # XXX Need to schedule subresource before and after some other tasks

        _, faces  = self._cmd_create_faces()
        _, routes = self._cmd_create_routes()

        cfg = list()
        cfg.append('add listener tcp local0 127.0.0.1 9695')
        cfg.extend(faces)
        cfg.extend(routes)

        self._config = TextFile(filename = self.config_file,
                node = self.node,
                owner = self,
                content = '\n'.join(cfg),
                overwrite = True)

        self._state.manager.commit_resource(self._config)

        start_or_restart = self.__method_restart__()

        return wait_resource_task(self._config) > start_or_restart

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    # Force local management of faces and routes

    _add_faces = None
    _remove_faces = None
    _get_faces = None
    _set_faces = None

    _add_routes = None
    _remove_routes = None
    _get_routes = None
    _set_routes = None

    #--------------------------------------------------------------------------
    # Method helpers
    #--------------------------------------------------------------------------

    def _start_as_daemon(self):
        """
        Start the metis forwarder as normal daemon
        """

        command = METIS_DAEMON_BOOTSTRAP
        args = {'port' : self.port, 'log_file' : self.log_file, 
            'cs_size' : self.cache_size, 'config' : self.config_file}
        return BashTask(self.node, command, parameters = args)

    def _restart_as_daemon(self):
        """
        Restart the metis forwarder as normal daemon
        """

        command = METIS_DAEMON_STOP + '; ' + METIS_DAEMON_BOOTSTRAP
        args = {'port' : self.port, 'log_file' : self.log_file, 
            'cs_size' : self.cache_size, 'config' : self.config_file}
        return BashTask(self.node, command, parameters = args)

    def _start_as_service(self):
        """
        Start the metis forwarder as service managed by systemd
        """
        return super().__method_start__()

    def _restart_as_service(self):
        """
        Restart the metis forwarder as service managed by systemd
        """
        return super().__method_restart__()

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    def __method_start__(self):
        return self._start_as_service()

    def __method_restart__(self):
        return self._restart_as_service()

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _cmd_create_faces(self):
        """Returns the list of commands used to update faces (delete and
        create).

        We need two lists because some delete might need to occur before create.
        This function is used to populate the config file and also alter the
        configuration of the forwarder live. It might be possible to further
        optimize but keeping the two separate seems important, since delete is
        only used for an already running metis.
        """
        create_cmds = list()
        delete_cmds = list()

        for face in self.faces:
            listener = MetisListener.listener_from_face(face)
            if listener not in self._listeners:
                self._listeners.append(listener)
                conn_id = self._listeners_idx
                self._listeners_idx += 1
                cmd = listener.get_setup_command(conn_id)
                create_cmds.append(cmd)

            face.id = 'conn{}'.format(self._nb_conn)
            self._nb_conn += 1

            if face.protocol is FaceProtocol.ether:
                assert isinstance(face, L2Face), \
                       'Ethernet face should be instance of L2Face'
                cmd = CMD_ADD_CONNECTION_ETHER.format(face = face)

            elif face.protocol in (FaceProtocol.tcp4, FaceProtocol.tcp6):
                assert isinstance(face, L4Face), \
                        "TCP/UDP face should be instance of L4Face"
                cmd = CMD_ADD_CONNECTION_L4.format(face = face, 
                        protocol = 'tcp')

            elif face.protocol in (FaceProtocol.udp4, FaceProtocol.udp6):
                assert isinstance(face, L4Face), \
                        'TCP/UDP face should be instance of L4Face'
                cmd = CMD_ADD_CONNECTION_L4.format(face = face, 
                        protocol = 'udp')

            else:
                raise ValueError('Unsupported face type for Metis')

            create_cmds.append(cmd)

        return (delete_cmds, create_cmds)

    def _cmd_create_routes(self):
        create_cmds = list()
        delete_cmds = list()
        for route in self.routes:
            cmd = CMD_ADD_ROUTE.format(route = route)
            create_cmds.append(cmd)
        return (delete_cmds, create_cmds)

    def _task_create_faces(self):
        delete_cmds, create_cmds = self._cmd_create_faces()

        delete_task = EmptyTask()
        if len(delete_cmds) > 0:
            cmds = '\n'.join('{} {}'.format(self._baseline, command) 
                    for command in delete_cmds)
            delete_task = BashTask(self.node, cmds)

        create_task = EmptyTask()
        if len(create_cmds) > 0:
            cmds = '\n'.join('{} {}'.format(self._baseline, command) 
                    for command in create_cmds)
            create_task = BashTask(self.node, cmds)

        return delete_task > create_task

    def _task_create_routes(self):
        delete_cmds, create_cmds = self._cmd_create_routes()

        delete_task = EmptyTask()
        if len(delete_cmds) > 0:
            delete_task = BashTask(self.node, "\n".join(delete_cmds))

        create_task = EmptyTask()
        if len(create_cmds) > 0:
            create_task = BashTask(self.node, '\n'.join(create_cmds))

        return delete_task > create_task
  
    def _write_environment_file(self):
        param_port = "PORT={port}"
        param_log_file = "LOG_FILE={log_file}"
        param_cs_capacity = "CS_SIZE={cs_size}"
        param_config = "CONFIG={config}"

        env = [param_port.format(port = self.port), 
               param_log_file.format(log_file = self.log_file),
               param_cs_capacity.format(cs_size = self.cache_size), 
               param_config.format(config = self.config_file)]

        environment_file = TextFile(filename = METIS_ETC_DEFAULT,
                                    node = self.node,
                                    owner = self,
                                    overwrite = True,
                                    content = '\n'.join(env))
        return environment_file
