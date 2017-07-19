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

from netmodel.model.type                    import String, Integer
from vicn.core.attribute	            import Attribute, Reference
from vicn.core.task                         import BashTask, inherit_parent
from vicn.resource.linux.file               import File
from vicn.resource.linux.package_manager    import Packages

METIS_KEYSTORE_CREATE = ('parc-publickey -c {filename} {password} '
                         '{subject_name} {size} {validity}')

# FIXME default passwords, not very sensitive
DEFAULT_KEYSTORE_FILE = "keystore.pkcs12"
DEFAULT_KEYSTORE_PASSWD = "password"
DEFAULT_KEYSTORE_VALIDITY = 365
DEFAULT_KEYSTORE_SUBJ = "password"
DEFAULT_KEYSTORE_KEYLENGTH = 2048

class MetisKeystore(File):
    """
    Resource: MetisKeystore
    """

    filename = Attribute(String, description = "File containing the keystore",
            default = DEFAULT_KEYSTORE_FILE, mandatory=False)
    password = Attribute(String,
            description = "Password for the keystore file",
            default = DEFAULT_KEYSTORE_PASSWD)
    subject_name = Attribute(String,
            description = "Subject name for the keystore",
            default = DEFAULT_KEYSTORE_SUBJ)
    validity = Attribute(String,
            description = "Validity period of the keystore",
            default = DEFAULT_KEYSTORE_VALIDITY)
    size = Attribute(Integer, description = 'Length of the keys',
            default = DEFAULT_KEYSTORE_KEYLENGTH)

    __package_names__ = ['libparc']

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __subresources__(self):
        packages = Packages(node=Reference(self, 'node'),
                names=self._get_package_names(), owner=self)
        return packages

    @inherit_parent
    def __create__(self):
        args = {'filename' : self.filename, 'password' : self.password,
                'subject_name' : self.subject_name, 'validity' : self.validity,
                'size' : self.size}
        return BashTask(self.node, METIS_KEYSTORE_CREATE, args)

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _get_package_names(self):
        package_names = list()
        for base in self.__class__.mro():
            if not '__package_names__' in vars(base):
                continue
            package_names.extend(getattr(base, '__package_names__'))
        return package_names


    def format_baseline(self, baseline):
        return baseline.format(keystore_file=self.filename, password=self.password)


