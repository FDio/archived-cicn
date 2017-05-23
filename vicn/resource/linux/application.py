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

from vicn.core.attribute                    import Reference
from vicn.core.resource                     import Resource, EmptyResource
from vicn.resource.application              import Application
from vicn.resource.linux.package_manager    import Packages

class LinuxApplication(Application):
    """
    Resource: Linux Application

    This resource ensures that the application is present on the system, and
    installs it during setup if necessary.
    """

    def __subresources__(self):
        package_names = self._get_package_names()
        if package_names:
            packages = Packages(node=Reference(self, 'node'),
                    names=package_names,
                    owner=self)
        else:
            packages = EmptyResource()

        process = None

        return packages > process

    #--------------------------------------------------------------------------
    # Private methods
    #--------------------------------------------------------------------------

    def _get_package_names(self):
        package_names = list()
        for base in self.__class__.mro():
            if not '__package_names__' in vars(base):
                continue
            package_names.extend(getattr(base, '__package_names__'))
        return package_names
