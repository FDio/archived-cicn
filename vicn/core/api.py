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
import json
import logging
import resource as ulimit
import sys

from netmodel.model.query       import Query
from netmodel.model.query       import ACTION_SELECT, ACTION_INSERT
from netmodel.model.query       import ACTION_UPDATE, ACTION_SUBSCRIBE
from netmodel.network.interface import InterfaceState
from netmodel.util.singleton    import Singleton
from vicn.core.exception        import NotConfigured
from vicn.core.resource_mgr     import ResourceManager
from vicn.resource.node         import Node

DEFAULT_SETTINGS = {
    'network': '192.168.0.0/16',
    'mac_address_base': '0x00163e000000',
    'websocket_port': 9999
}

log = logging.getLogger(__name__)

class Event_ts(asyncio.Event):
    def set(self):
        self._loop.call_soon_threadsafe(super().set)

#------------------------------------------------------------------------------

class API(metaclass = Singleton):

    def terminate(self):
        ResourceManager().terminate()

    def parse_topology_file(self, topology_fn):
        log.debug("Parsing topology file %(topology_fn)s" % locals())
        try:
            topology_fd = open(topology_fn, 'r')
        except IOError:
            self.error("Topology file '%(topology_fn)s not found" % locals())
            return None

        try:
            topology = json.loads(topology_fd.read())

            # SETTING
            settings = DEFAULT_SETTINGS
            settings.update(topology.get('settings'))

            # VICN process-related initializations
            nofile = settings.get('ulimit-n', None)
            if nofile is not None and nofile > 0:
                if nofile < 1024:
                    log.error('Too few allowed open files for the process')
                    import os; os._exit(1)

                log.info('Setting open file descriptor limit to {}'.format(
                            nofile))
                ulimit.setrlimit(
                        ulimit.RLIMIT_NOFILE,
                        (nofile, nofile))

            ResourceManager(base=topology_fn, settings=settings)

            # NODES
            resources = topology.get('resources', list())
            for resource in resources:
                try:
                    ResourceManager().create_from_dict(**resource)
                except Exception as e:
                    log.warning("Could not create resource '%r': %r" % \
                            (resource, e,))
                    import traceback; traceback.print_exc()
                    continue

        except SyntaxError:
            log.error("Error reading topology file '%s'" % (topology_fn,))
            sys.exit(1)

        log.debug("Done parsing topology file %(topology_fn)s" % locals())

    def configure(self, name, setup=False):
        log.info("Parsing configuration file", extra={'category': 'blue'})
        self.parse_topology_file(name)
        self._configured = True
        ResourceManager().setup(commit=setup)

    def setup(self):
        if not self._configured:
            raise NotConfigured
        ResourceManager().setup()

    def teardown(self):
        ResourceManager().teardown()

    def open_terminal(self, node_name):
        node = ResourceManager().by_name(node_name)
        assert isinstance(node, Node)

        node.open_terminal()
