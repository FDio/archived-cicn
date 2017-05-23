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
import os

from netmodel.model.type                import Double, String, Self
from vicn.core.address_mgr              import AddressManager
from vicn.core.attribute                import Attribute
from vicn.core.resource                 import Resource

log = logging.getLogger(__name__)

DEFAULT_USERNAME = 'root'
DEFAULT_SSH_PRIVATE_KEY = os.path.expanduser(os.path.join(
        '~', '.vicn', 'ssh_client_cert', 'ssh_client_key'))
DEFAULT_SSH_PUBLIC_KEY = os.path.expanduser(os.path.join(
        '~', '.vicn', 'ssh_client_cert', 'ssh_client_key.pub'))

class Node(Resource):
    """
    Resource: Node
    """

    x = Attribute(Double, description = 'X coordinate',
            default = 0.0)
    y = Attribute(Double, description = 'Y coordinate',
            default = 0.0)
    category = Attribute(String)
    os = Attribute(String, description = 'OS',
            default = 'ubuntu',
            choices = ['debian', 'ubuntu'])
    dist = Attribute(String, description = 'Distribution name',
            default = 'xenial',
            choices = ['trusty', 'xenial', 'sid'])
    arch = Attribute(String, description = 'Architecture',
            default = 'amd64',
            choices = ['amd64'])
    node_with_kernel = Attribute(Self,
            description = 'Node on which the kernel sits',
            ro = True)

    #---------------------------------------------------------------------------
    # Constructor and Accessors
    #---------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._management_interface = None

    #---------------------------------------------------------------------------
    # Public API
    #---------------------------------------------------------------------------

    @property
    def management_interface(self):
        if not self._management_interface:
            raise Exception("No management interface has been defined")
        return self._management_interface

    def execute(self, command, output = False, as_root = False):
        raise NotImplementedError
