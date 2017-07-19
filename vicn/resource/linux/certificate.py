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
from vicn.core.task             import inherit_parent
from vicn.resource.linux.file   import File
from vicn.resource.node         import Node

DEFAULT_RSA_LENGTH = '4096'
DEFAULT_SUBJECT = '/CN=www.cisco.com/L=Paris/O=Cisco/C=FR'

CMD_CREATE='\n'.join([
    '# Generate a new certificate',
    'mkdir -p $(dirname {self.key})',
    'mkdir -p $(dirname {self.cert})',
    'openssl req -x509 -newkey rsa:' + DEFAULT_RSA_LENGTH  +
    ' -keyout {self.key} -out {self.cert} -subj ' + DEFAULT_SUBJECT + ' -nodes'
])

class Certificate(Resource):
    """
    Resource: Certificate

    Implements a SSL certificate.
    """
    node = Attribute(Node,
            description = 'Node on which the certificate is created',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne)
    cert = Attribute(String, description = 'Certificate path',
            mandatory = True)
    key = Attribute(String, description = 'Key path')

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._cert_file = File(node = Reference(self, 'node'),
                filename = Reference(self, 'cert'),
                managed = False)
        if self.key:
            self._key_file = File(node = Reference(self, 'node'),
                    filename = Reference(self, 'key'),
                    managed = False)
        else:
            self._key_file = None

    @inherit_parent
    def __get__(self):
        if self.key:
            return self._cert_file.__get__() | self._key_file.__get__()
        else:
            return self._cert_file.__get__()

    @inherit_parent
    def __create__(self):
        return BashTask(self.node, CMD_CREATE, {'self': self})

    @inherit_parent
    def __delete__(self):
        if self.key:
            return self._cert_file.__delete__() | self._key_file.__delete__()
        else:
            return self._cert_file.__delete__()


