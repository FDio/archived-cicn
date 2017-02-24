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

class Singleton(type):
    """
    Classes that inherit from Singleton can be instanciated only once 

    See also
    http://stackoverflow.com/questions/6760685/creating-a-singleton-in-python
    """

    def __init__(cls, name, bases, dic):
        super(Singleton,cls).__init__(name,bases,dic)
        cls.instance=None

    def __call__(cls, *args, **kw):
        if cls.instance is None:
            cls.instance=super(Singleton,cls).__call__(*args,**kw)
        return cls.instance

    def _drop(cls):
        "Drop the instance (for testing purposes)."
        if cls.instance is not None:
            del cls.instance
            cls.instance = None

