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

from netmodel.model.type                import Integer, String, Bool
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.requirement              import Requirement
from vicn.core.task                     import BashTask
from vicn.resource.icn.icn_application  import ICNApplication
from vicn.resource.icn.icn_application  import ICN_SUITE_CCNX_1_0
from vicn.resource.icn.producer         import Producer
from vicn.resource.icn.consumer         import Consumer
from vicn.resource.node                 import Node

DEFAULT_PING_PAYLOAD_SIZE = 64
DEFAULT_PING_COUNT = 100

class IPing(ICNApplication):
    """
    Resource: IPingClient
    """

    __package_names__ = ["libicnet"]

    prefixes = Attribute(String,
            description = "name served by the ping server",
            default = lambda self: self.default_name(),
            mandatory = False,
            multiplicity = Multiplicity.OneToMany)
    node = Attribute(Node,
            requirements=[
                Requirement("forwarder",
                    capabilities = set(['ICN_SUITE_CCNX_1_0']),
                    properties = {"protocol_suites" : ICN_SUITE_CCNX_1_0})
            ])

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    def __method_start__(self):
        return self._build_command()

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def default_name(self):
        return ['/iping']

    def _def_protocol_suite(self):
        return ICN_SUITE_CCNX_1_0

#------------------------------------------------------------------------------

class IPingClient(IPing, Producer):
    """
    Resource: IPingClient
    """

    flood = Attribute(Bool, description = 'enable flood mode',
            default = False)
    count = Attribute(Integer, description = 'number of ping to send')
    interval = Attribute(Integer,
            description = 'interval between interests in ping mode')
    size = Attribute(Integer, description = 'size of the interests')

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _build_command(self):
        template = ["iPing_Client", "-l ccnx:{prefix}"]
        params={'prefix' : self.prefixes[0]}

        if self.flood:
            template.append("-f")
        else:
            template.append("-p") #Ping mode

        if self.count:
            template.append("-c {count}")
            params["count"] = self.count
        if self.size:
            template.append("-s {size}")
            params['size'] = self.size
        if self.interval:
            template.append("-i {interval}")
            params['interval'] = self.interval

        return BashTask(self.node, ' '.join(template), parameters=params)

#------------------------------------------------------------------------------

class IPingServer(IPing, Consumer):

    size = Attribute(Integer, description = "size of the payload")

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _build_command(self):
        template = ["iPing_Server", "-l ccnx:{prefix}"]
        params={'prefix' : self.prefixes[0]}

        if self.size:
            template.append("-s {size}")
            params['size'] = self.size

        return BashTask(self.node, ' '.join(template), parameters=params)
