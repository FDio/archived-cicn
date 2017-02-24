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

import logging
import re

from vicn.core.exception                import ResourceNotFound
from vicn.core.task                     import inline_task, BashTask
from vicn.core.task                     import ParseRegexTask
from vicn.resource.icn.forwarder        import Forwarder
from vicn.resource.icn.icn_application  import ICN_SUITE_NDN

log = logging.getLogger(__name__)

NFD_CONF_FILE = "/etc/ndn/nfd.conf"

CMD_SET_STRATEGY_CACHE = '\n'.join([
    'sed -i "s/^.*cs_max_packets .*$/  cs_max_packets {nfd.cache_size}/" ' \
        '{conf_file}',
    'sed -i "0,/\/ / s/\/localhost\/nfd\/strategy\/.*/' \
        '\/localhost\/nfd\/strategy\/{nfd.fw_strategy}/" {conf_file}',
    'service nfd restart'])
CMD_RESET_CACHE = '''
sed -i "s/^.*cs_max_packets .*$/  cs_max_packets 65536/" {conf_file}
service nfd restart
'''

CMD_ADD_ROUTE = 'nfdc register {route.prefix} {route.face.nfd_uri}'
# or: nfdc register {route.prefix} {route.face.id}

CMD_REMOVE_ROUTE = 'nfdc unregister {route.prefix} {route.face.nfd_uri}'
# or: nfdc unregister {route.prefix} {route.face.id}

CMD_ADD_FACE = 'nfdc create {face.nfdc_flags} {face.nfd_uri}'

CMD_REMOVE_FACE = 'nfdc destroy {face.id}'
# or: nfdc destroy {face.nfd_uri}

# FIXME redundant with Forwarder.FaceType
layer_2_protocols = ["udp", "udp4", "tcp", "tcp4", "ether"]

NFD_DEFAULT_PORT = 6363

# Regular expressions used for parsing nfdc results
STR_ADD_FACE = ('Face creation succeeded: ControlParameters\(FaceId: '
        '(?P<id>.*?), Uri: (?P<face_uri>.*?), \)')
RX_ADD_FACE  = re.compile(STR_ADD_FACE)

class NFD(Forwarder):
    """
    Resource: NFD
    """

    __capabilities__ = set(['ICN_SUITE_NDN'])
    __service_name__ = 'nfd'
    __package_names__ = ['nfd']

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __get__(self):
        # NFD is assumed not to exist
        raise ResourceNotFound

    def __create__(self):
        # Modify the configuration file before running the forwarder service
        conf = BashTask(self.node, CMD_SET_STRATEGY_CACHE, {'nfd': self})
        forwarder = Forwarder.__create__(self)
        return conf.then(forwarder)

    def __delete__(self):
        raise NotImplementedError

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    @inline_task
    def _get_routes(self):
        return {'routes': list()}

    @inline_task
    def _add_routes(self, route):
        return BashTask(self.node, CMD_ADD_ROUTE, {'route': route})

    @inline_task
    def _remove_routes(self, route):
        return BashTask(self.node, CMD_REMOVE_ROUTE, {'route': route})

    @inline_task
    def _get_faces(self):
        return {'faces': list()}

    @inline_task
    def _add_faces(self, face):
        add_face = BashTask(self.node, CMD_ADD_FACE, {'face': face})
        set_face_id = ParseRegexTask(RX_ADD_FACE)
        return add_face.compose(set_face_id)

    @inline_task
    def _remove_faces(self, face):
        return BashTask(self.node, CMD_REMOVE_FACE, {'face': face})

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    def __method_reset_cache__(self, conf_file):
        return BashTask(self.node, CMD_RESET_CACHE, {'conf_file': conf_file})

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _get_default_port(self):
        return NFD_DEFAULT_PORT

    def _def_protocol_suite(self):
        return ICN_SUITE_NDN
