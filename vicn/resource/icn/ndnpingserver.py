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

from netmodel.model.type            import String
from vicn.core.attribute            import Attribute
from vicn.core.requirement          import Requirement
from vicn.core.task                 import BashTask
from vicn.resource.icn.producer     import Producer
from vicn.resource.linux.service    import Service

TPL_DEFAULT_PREFIX='/ndn/{node.name}'

FN_ETC_DEFAULT='/etc/default/ndnping'

TPL_ETC_DEFAULT='''
# defaults for ndnping server

# Prefix should be set to a valid value
PREFIX="/ndn/server"

FLAGS=""
'''

CMD_START = 'ndnpingserver {prefix} &'

class NDNPingServerBase(Producer):
    """NDNPingServer Resource

    This NDNPingServer resource wraps a NDN ping server

    Attributes:
        prefixes (List[str]) : (overloaded) One-element list containing the
            prefix on which the ping server is listening.

    TODO:
      - ndnpingserver only supports a single prefix.
    """
    prefixes = Attribute(String,
            default = lambda self: self._default_prefixes())

    node = Attribute(requirements = [
            Requirement("forwarder", 
                capabilities = set(['ICN_SUITE_CCNX_1_0'])) ])

    __package_names__ = ['ndnping']

    def _default_prefixes(self):
        return [self.format(TPL_DEFAULT_PREFIX)]

#------------------------------------------------------------------------------

class NDNPingServer(NDNPingServerBase):

    def __method_start__(self):
        return BashTask(self.node, CMD_START)

#------------------------------------------------------------------------------

class NDNPingService(NDNPingServerBase, Service):
    __package_names__ = ['ndnping']
    __service_name__ = 'ndnping'
