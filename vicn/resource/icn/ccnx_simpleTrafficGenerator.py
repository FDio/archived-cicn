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
from vicn.core.resource                 import Resource, EmptyResource
from vicn.core.task                     import EmptyTask, inherit_parent
from vicn.resource.icn.icn_application  import ICN_SUITE_CCNX_1_0
from vicn.resource.node                 import Node

from vicn.resource.icn.ccnx_consumer_producer_test  import CcnxConsumerTest
from vicn.resource.icn.ccnx_consumer_producer_test  import CcnxProducerTest

class CcnxSimpleTrafficGenerator(Resource):

    prefix = Attribute(String,
            description = "Routable prefix for the applications",
            default = lambda self: self.default_name(),
            mandatory = False)
    consumers = Attribute(Node,
            multiplicity = Multiplicity.OneToMany)
    producers = Attribute(Node,
            multiplicity = Multiplicity.OneToMany)

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._sr = None

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inherit_parent
    def __subresources__(self):
        """
        Create the list of consumers and producers.
        For each of them, assign a different namespace under the same prefix.
        """

        sr = EmptyResource()
        for producer in self.producers:
            producer = CcnxProducerTest(node = producer,
                    owner = self,
                    prefixes = [self.prefix])
            sr = sr | producer
        for consumer in self.consumers:
            full_prefix = self.prefix
            consumer = CcnxConsumerTest(node = consumer,
                    owner = self,
                    prefixes = [full_prefix])
            sr = sr | consumer
        self._sr = sr
        return sr

    #--------------------------------------------------------------------------
    # Methods
    #--------------------------------------------------------------------------

    def __method_start__(self):
        if self._sr is None:
            return

        tasks = EmptyTask()
        for sr in self._sr:
            sr_task = sr.__method_start__()
            tasks = tasks | sr_task
        return tasks

    def __method_stop__(self):
        if self._sr is None:
            return

        tasks = EmptyTask()
        for sr in self._sr:
            sr_task = sr.__method_stop__()
            tasks = tasks | sr_task
        return tasks

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def default_name(self):
        return ['/ccnxtest']

    def _def_protocol_suite(self):
        return ICN_SUITE_CCNX_1_0

