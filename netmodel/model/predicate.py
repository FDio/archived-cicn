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

from netmodel.model.field_names import FieldNames, FIELD_SEPARATOR

from operator import (
    and_, or_, inv, add, mul, sub, mod, truediv, lt, le, ne, gt, ge, eq, neg
)

# Define the inclusion operators
class contains(type): pass
class included(type): pass

class Predicate:

    operators = {
        '=='       : eq,
        '!='       : ne,
        '<'        : lt,
        '<='       : le,
        '>'        : gt,
        '>='       : ge,
        'CONTAINS' : contains,
        'INCLUDED' : included
    }

    operators_short = {
        '=' : eq,
        '~' : ne,
        '<' : lt,
        '[' : le,
        '>' : gt,
        ']' : ge,
        '}' : contains,
        '{' : included
    }

    def __init__(self, *args, **kwargs):
        """
        Build a Predicate instance.
        Args: 
            kwargs: You can pass:
              - 3 args (left, operator, right)
                  left: The left operand (it may be a String instance or a
                      tuple)
                  operator: See Predicate.operators, this is the binary
                      operator involved in this Predicate. 
                  right: The right value (it may be a String instance
                      or a literal (String, numerical value, tuple...))
              - 1 argument (list or tuple), containing three arguments
                (variable, operator, value)
        """
        if len(args) == 3:
            key, op, value = args
        elif len(args) == 1 and isinstance(args[0], (tuple, list)) and \
                len(args[0]) == 3:
            key, op, value = args[0]
        elif len(args) == 1 and isinstance(args[0], Predicate):
            key, op, value = args[0].get_tuple()
        else:
            raise Exception("Bad initializer for Predicate (args = %r)" % args)

        assert not isinstance(value, (frozenset, dict, set)), \
                "Invalid value type (type = %r)" % type(value)
        if isinstance(value, list):
            value = tuple(value)

        self.key = key
        if isinstance(op, str):
            op = op.upper()
        if op in self.operators.keys():
            self.op = self.operators[op]
        elif op in self.operators_short.keys():
            self.op = self.operators_short[op]
        else:
            self.op = op

        if isinstance(value, list):
            self.value = tuple(value)
        else:
            self.value = value

    def __str__(self):
        """
        Returns:
            The '%s' representation of this Predicate.
        """
        return repr(self)

    def __repr__(self):
        """
        Returns:
            The '%r' representation of this Predicate.
        """
        key, op, value = self.get_str_tuple()
        if isinstance(value, (tuple, list, set, frozenset)):
            value = [repr(v) for v in value]
            value = "(%s)" % ", ".join(value)
        return "%s %s %r" % (key, op, value) 

    def __hash__(self):
        """
        Returns:
            The hash of this Predicate (this allows to define set of
            Predicate instances).
        """
        return hash(self.get_tuple())

    def __eq__(self, predicate):
        """
        Returns:
            True iif self == predicate.
        """
        if not predicate:
            return False
        return self.get_tuple() == predicate.get_tuple()

    def copy(self):
        return copy.deepcopy(self)

    def get_key(self):
        """
        Returns:
            The left operand of this Predicate. It may be a String
            or a tuple of Strings.
        """
        return self.key
    
    def set_key(self, key):
        """
        Set the left operand of this Predicate.
        Params:
            key: The new left operand.
        """
        self.key = key

    def update_key(self, function):
        self.set_key(function(self.get_key()))

    def get_op(self):
        return self.op

    def set_op(self, op):
        self.op = op

    def get_value(self):
        return self.value

    def set_value(self, value):
        self.value = value

    def get_tuple(self):
        return (self.key, self.op, self.value)

    def get_tuple_ext(self):
        key, op, value = self.get_tuple()
        key_field, _, key_subfield = key.partition(FIELD_SEPARATOR)
        return (key_field, key_subfield, op, value)

    def get_str_op(self):
        op_str = [s for s, op in self.operators.items() if op == self.op]
        return op_str[0]

    def get_str_tuple(self):
        return (self.key, self.get_str_op(), self.value,)

    def to_list(self):
        return list(self.get_str_tuple())

    def match(self, dic, ignore_missing=False):
        # Can we match ?
        if self.key not in dic:
            return ignore_missing

        if self.op == eq:
            if isinstance(self.value, list):
                return (dic[self.key] in self.value) 
            else:
                return (dic[self.key] == self.value)
        elif self.op == ne:
            if isinstance(self.value, list):
                return (dic[self.key] not in self.value) 
            else:
                return (dic[self.key] != self.value) 
        elif self.op == lt:
            if isinstance(self.value, str):
                # prefix match
                return dic[self.key].startswith('%s.' % self.value)
            else:
                return (dic[self.key] < self.value)
        elif self.op == le:
            if isinstance(self.value, str):
                return dic[self.key] == self.value or \
                        dic[self.key].startswith('%s.' % self.value)
            else:
                return (dic[self.key] <= self.value)
        elif self.op == gt:
            if isinstance(self.value, str):
                # prefix match
                return self.value.startswith('%s.' % dic[self.key])
            else:
                return (dic[self.key] > self.value)
        elif self.op == ge:
            if isinstance(self.value, str):
                # prefix match
                return dic[self.key] == self.value or \
                        self.value.startswith('%s.' % dic[self.key])
            else:
                return (dic[self.key] >= self.value)
        elif self.op == and_:
            return (dic[self.key] & self.value) 
        elif self.op == or_:
            return (dic[self.key] | self.value) 
        elif self.op == contains:
            try:
                method, subfield = self.key.split('.', 1)
                return not not [ x for x in dic[method] \
                        if x[subfield] == self.value] 
            except ValueError: # split has failed
                return self.value in dic[self.key]
        elif self.op == included:
            return dic[self.key] in self.value
        else:
            raise Exception("Unexpected table format: %r" % dic)

    def filter(self, dic):
        """
        Filter dic according to the current predicate.
        """

        if '.' in self.key:
            # users.hrn
            method, subfield = self.key.split('.', 1)
            if not method in dic:
                return None 

            if isinstance(dic[method], dict):
                subpred = Predicate(subfield, self.op, self.value)
                match = subpred.match(dic[method])
                return dic if match else None

            elif isinstance(dic[method], (list, tuple)):
                # 1..N relationships
                match = False
                if self.op == contains:
                    return dic if self.match(dic) else None
                else:
                    subpred = Predicate(subfield, self.op, self.value)
                    dic[method] = subpred.filter(dic[method])
                    return dic
            else:
                raise Exception("Unexpected table format: %r", dic)


        else:
            # Individual field operations
            return dic if self.match(dic) else None

    def get_field_names(self):
        if isinstance(self.key, (list, tuple, set, frozenset)):
            return FieldNames(self.key)
        else:
            return FieldNames([self.key])

    def get_value_names(self):
        if isinstance(self.value, (list, tuple, set, frozenset)):
            return FieldNames(self.value)
        else:
            return FieldNames([self.value])

    def has_empty_value(self):
        if isinstance(self.value, (list, tuple, set, frozenset)):
            return not any(self.value)
        else:
            return not self.value

    def is_composite(self):
        """
        Returns:
            True iif this Predicate instance involves
            a tuple key (and tuple value).
        """ 
        return isinstance(self.get_key(), tuple)
    
    def rename(self, aliases):
        if self.is_composite():
            raise NotImplemented
        if self.key in aliases:
            self.key = aliases[self.key]
