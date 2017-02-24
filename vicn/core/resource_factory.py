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

import copy
import inspect
import logging
import pkgutil
import traceback

from netmodel.model.type               import Type
from netmodel.util.singleton           import Singleton

log = logging.getLogger(__name__)

# Blacklist : resource that are temporarily disabled loaded
RESOURCE_BLACKLIST = ['LinuxBridge']

class ResourceFactory(metaclass=Singleton):
    """
    This manages classes, not instances
    """

    def __init__(self):
        self._registry = dict()
        self._register_all()

    def _register_all(self):
        log.info("Registering resources")
        from vicn.core.resource import Resource

        from vicn import resource as package
        prefix = package.__name__ + "."

        # Because aggregates might not be instanciated in order, we accumulate
        # them and register them at the end
        delayed_aggregates = dict()

        # Explored modules are automatically imported by walk_modules + it
        # allows to explore recursively resources/
        # http://docs.python.org/2/library/pkgutil.html
        for importer, modname, ispkg in pkgutil.walk_packages(package.__path__,
                prefix, onerror = None):
            try:
                module = __import__(modname, fromlist = "dummy")

                classes = [m[1] for m in inspect.getmembers(module, 
                        inspect.isclass) if m[1].__module__ == modname]
                for cls in classes:
                    if not issubclass(cls, Resource):
                        continue

                    if cls.__name__ in RESOURCE_BLACKLIST:
                        print('Skipped blacklisted resource ' + cls.__name__)
                        continue

                    # Register module to resource factory
                    self._registry[cls.__qualname__] = cls
                    Type._registry[cls.__qualname__.lower()] = cls

            except Exception as e:
                log.warning("Cannot load %s : %s: %s" % (modname, e, 
                            traceback.format_exc()))

        log.info("Registered resources are: {%s}" % ", ".join(sorted(
                        self._registry.keys())))

    def get_available_resources(self):
        return self._registry

