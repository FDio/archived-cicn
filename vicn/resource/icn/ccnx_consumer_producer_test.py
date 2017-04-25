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

from netmodel.model.type                import String
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.requirement              import Requirement
from vicn.core.task                     import BashTask
from vicn.resource.icn.icn_application  import ICN_SUITE_CCNX_1_0
from vicn.resource.icn.consumer         import Consumer
from vicn.resource.icn.producer         import Producer
from vicn.resource.node                 import Node

class CcnxConsumerTest(Consumer):
    """
    Resource: CcnxConsumerTest

    Test consumer exchanging dummy data.
    """

    __package_names__ = ["libicnet"]

    prefixes = Attribute(String,
            description = "Name served by the producer server test",
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
    # Internal methods
    #--------------------------------------------------------------------------

    def default_name(self):
        return ['/ccnxtest']

    def _def_protocol_suite(self):
        return ICN_SUITE_CCNX_1_0

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    def __method_start__(self):
        template = ["consumer-test", " ccnx:{prefix}"]
        params = {'prefix' : self.prefixes[0]}
        return BashTask(self.node, ' '.join(template), parameters = params)

    def __method_stop__(self):
        raise NotImplementedError

#------------------------------------------------------------------------------

class CcnxProducerTest(Producer):
    """
    Resource: CcnxConsumerTest

    Test producer exchanging dummy data.
    """

    __package_names__ = ["libicnet"]

    node = Attribute(Node,
        requirements = [Requirement("forwarder",
            capabilities = set(['ICN_SUITE_CCNX_1_0']),
            properties = {"protocol_suites" : ICN_SUITE_CCNX_1_0})])

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def default_name(self):
        return ['/ccnxtest']

    def _def_protocol_suite(self):
        return ICN_SUITE_CCNX_1_0

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    def __method_start__(self):
        template = ["producer-test", " ccnx:{prefix}"]
        params = {'prefix' : self.prefixes[0]}

        return BashTask(self.node, ' '.join(template), parameters = params)

    def __method_stop__(self):
        raise NotImplementedError

