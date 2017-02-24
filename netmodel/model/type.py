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

from netmodel.util.meta import inheritors

class BaseType:
    @staticmethod
    def name():
        return self.__class__.__name__.lower()

class String(BaseType):
    def __init__(self, *args, **kwargs):
        self._min_size = kwargs.pop('min_size', None)
        self._max_size = kwargs.pop('max_size', None)
        self._ascii = kwargs.pop('ascii', False)
        self._forbidden = kwargs.pop('forbidden', None)
        super().__init__()

class Integer(BaseType):
    def __init__(self, *args, **kwargs):
        self._min_value = kwargs.pop('min_value', None)
        self._max_value = kwargs.pop('max_value', None)
        super().__init__()
    
class Double(BaseType):
    def __init__(self, *args, **kwargs):
        self._min_value = kwargs.pop('min_value', None)
        self._max_value = kwargs.pop('max_value', None)
        super().__init__()

class Bool(BaseType):
    pass

class Dict(BaseType):
    pass

class Self(BaseType):
    """Self-reference
    """

class Type:
    BASE_TYPES = (String, Integer, Double, Bool) 
    _registry = dict()

    @staticmethod
    def from_string(type_name, raise_exception=True):
        """Returns a type corresponding to the type name.

        Params:
            type_name (str) : Name of the type

        Returns
            Type : Type class of the requested type name
        """
        type_cls = [t for t in Type.BASE_TYPES if t.name == type_name]
        if type_cls:
            return type_cls[0]

        type_cls = Type._registry.get(type_name, None)
        if not type_cls:
            raise Exception("No type found: {}".format(type_name))
        return type_cls

    @staticmethod
    def is_base_type(type_cls):
        return type_cls in Type.BASE_TYPES

    @staticmethod
    def exists(typ):
        return (isinstance(typ, type) and typ in inheritors(BaseType)) \
            or isinstance(typ, BaseType)

is_base_type = Type.is_base_type
is_type = Type.exists
