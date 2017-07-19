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

from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.resource                 import Resource
from vicn.resource.linux.certificate    import Certificate
from vicn.resource.node                 import Node

PACKAGE = 'ca-certificates'

CMD_ADD_CERTIFICATE = '''
cp {certificate.cert} {store.PATH}
dpkg-reconfigure ca-certificates
'''

class CertificateStore(Resource):
    """
    Resource: System-wide Certificate Store

    This resource allows manipulation of the trusted certificates on the system.
    Use with care.

    TODO:
     - Ensure ca-certificates package is installed.
     - Issue a warning to the user when it is used.
    """

    PATH = '/usr/share/ca-certificates'

    certificates = Attribute(Certificate, multiplicity = Multiplicity.OneToMany)
    node = Attribute(Node, mandatory = True, requirements = [
            #Requirement(PACKAGE, 'in', 'packages')
        ])

    def _add_certificate(self):
        # Return a task that takes a certificate as parameter
        PARAM = None
        return BashTask(self.node, CMD_ADD_CERTIFICATE, {'store': self, 'certificate': PARAM},
                root = True)
