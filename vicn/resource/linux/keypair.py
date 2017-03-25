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

import os.path

from netmodel.model.type        import String
from vicn.core.attribute        import Attribute, Multiplicity, Reference
from vicn.core.exception        import ResourceNotFound
from vicn.core.resource         import Resource
from vicn.core.task             import task, inline_task, BashTask
from vicn.resource.linux.file   import File
from vicn.resource.node         import Node

CMD_CREATE='''
mkdir -p {dirname}
ssh-keygen -t rsa -N "" -f {self.key}
'''

class Keypair(Resource):
    """
    Resource: Keypair

    Implements a SSH keypair
    """
    node = Attribute(Node, 
            description = 'Node on which the certificate is created',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne)
    key = Attribute(String, description = 'Key path',
            mandatory = True)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------
    
    @inline_task
    def __initialize__(self):
        self._pubkey_file = File(node = Reference(self, 'node'),
                filename = self.key + '.pub',
                managed = False)
        self._key_file = File(node = Reference(self, 'node'), 
                filename = self.key, 
                managed = False)

    def __get__(self):
        return self._pubkey_file.__get__() | self._key_file.__get__()

    def __create__(self):
        return BashTask(None, CMD_CREATE, {
                'dirname': os.path.dirname(self.key),
                'self': self})
    
    def __delete__(self):
        return self._pubkey_file.__delete__() | self._key_file.__delete__()


