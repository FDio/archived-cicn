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

from netmodel.model.field_names  import FieldNames
from netmodel.model.predicate    import Predicate, eq, included
from netmodel.util.misc         import is_iterable

class Filter(set):
    """
    A Filter is a set of Predicate instances
    """

    @staticmethod
    def from_list(l):
        """
        Create a Filter instance by using an input list.
        Args:
            l: A list of Predicate instances.
        """
        f = Filter()
        try:
            for element in l:
                f.add(Predicate(*element))
        except Exception as e:
            #print("Error in setting Filter from list", e)
            return None
        return f
        
    @staticmethod
    def from_dict(d):
        """
        Create a Filter instance by using an input dict.
        Args:
            d: A dict {key : value} instance where each
               key-value pair leads to a Predicate.
               'key' could start with the operator to be
               used in the predicate, otherwise we use
               '=' by default.
        """
        f = Filter()
        for key, value in d.items():
            if key[0] in Predicate.operators.keys():
                f.add(Predicate(key[1:], key[0], value))
            else:
                f.add(Predicate(key, '=', value))
        return f

    def to_list(self):
        """
        Returns:
            The list corresponding to this Filter instance.
        """
        ret = list() 
        for predicate in self:
            ret.append(predicate.to_list())
        return ret

    @staticmethod
    def from_clause(clause):
        """
        NOTE: We can only handle simple clauses formed of AND fields.
        """
        raise NotImplementedError

    @staticmethod
    def from_string(string):
        """
        """
        from netmodel.model.sql_parser import SQLParser
        p = SQLParser()
        ret = p.filter.parseString(string, parseAll=True)
        return ret[0] if ret else None

    def filter_by(self, predicate):
        """
        Update this Filter by adding a Predicate.
        Args:
            predicate: A Predicate instance.
        Returns:
            The resulting Filter instance.
        """
        assert isinstance(predicate, Predicate),\
            "Invalid predicate = %s (%s)" % (predicate, type(predicate))
        self.add(predicate)
        return self

    def unfilter_by(self, *args):
        assert len(args) == 1 or len(args) == 3, \
                "Invalid expression for filter"

        if not self.is_empty():
            if len(args) == 1: 
                # we got a Filter, or a set, or a list, or a tuple or None.
                filters = args[0]
                if filters != None:
                    if not isinstance(filters, (set, list, tuple, Filter)):
                        filters = [filters]
                    for predicate in set(filters):
                        self.remove(predicate)
            elif len(args) == 3: 
                # we got three args: (field_name, op, value)
                predicate = Predicate(*args)
                self.remove(predicate)

        assert isinstance(self, Filter),\
            "Invalid filters = %s" % (self, type(self))
        return self

    def add(self, predicate_or_filter):
        """
        Adds a predicate or a filter (a set of predicate) -- or a list thereof
        -- to the current filter.
        """
        if is_iterable(predicate_or_filter):
            map(self.add, predicate_or_filter)
            return

        assert isinstance(predicate_or_filter, Predicate)
        set.add(self, predicate_or_filter)

    def is_empty(self):
        """
        Tests whether this Filter is empty or not.
        Returns:
            True iif this Filter is empty.
        """
        return len(self) == 0

    def __str__(self):
        """
        Returns:
            The '%s' representation of this Filter.
        """
        if self.is_empty():
            return "<empty filter>"
        else:
            return " AND ".join([str(pred) for pred in self])

    def __repr__(self):
        """
        Returns:
            The '%r' representation of this Filter.
        """
        return '<Filter: %s>' % self 

    def __key(self):
        return tuple([hash(pred) for pred in self])

    def __hash__(self):
        return hash(self.__key())

    def __additem__(self, value):
        if not isinstance(value, Predicate):
            raise TypeError("Element of class Predicate expected, received %s"\
                    % value.__class__.__name__)
        set.__additem__(self, value)


    def copy(self):
        return copy.deepcopy(self)

    def keys(self):
        """
        Returns:
            A set of String corresponding to each field name
            involved in this Filter.
        """
        return set([x.key for x in self])

    def has(self, key):
        for x in self:
            if x.key == key:
                return True
        return False

    def has_op(self, key, op):
        for x in self:
            if x.key == key and x.op == op:
                return True
        return False

    def has_eq(self, key):
        return self.has_op(key, eq)

    def get(self, key):
        ret = []
        for x in self:
            if x.key == key:
                ret.append(x)
        return ret

    def delete(self, key):
        to_del = []
        for x in self:
            if x.key == key:
                to_del.append(x)
        for x in to_del:
            self.remove(x)
            
    def get_op(self, key, op):
        if isinstance(op, (list, tuple, set)):
            for x in self:
                if x.key == key and x.op in op:
                    return x.value
        else:
            for x in self:
                if x.key == key and x.op == op:
                    return x.value
        return None

    def get_eq(self, key):
        return self.get_op(key, eq)

    def set_op(self, key, op, value):
        for x in self:
            if x.key == key and x.op == op:
                x.value = value
                return
        raise KeyError(key)

    def set_eq(self, key, value):
        return self.set_op(key, eq, value)

    def get_predicates(self, key):
        ret = []
        for x in self:
            if x.key == key:
                ret.append(x)
        return ret

    def match(self, dic, ignore_missing=True):
        for predicate in self:
            if not predicate.match(dic, ignore_missing):
                return False
        return True

    def filter(self, l):
        output = []
        for x in l:
            if self.match(x):
                output.append(x)
        return output

    def get_field_names(self):
        field_names = FieldNames()
        for predicate in self:
            field_names |= predicate.get_field_names()
        return field_names

    def grep(self, fun):
        return Filter([x for x in self if fun(x)])

    def rgrep(self, fun):
        return Filter([x for x in self if not fun(x)])

    def split(self, fun, true_only = False):
        true_filter, false_filter = Filter(), Filter()
        for predicate in self:
            if fun(predicate):
                true_filter.add(predicate)
            else:
                false_filter.add(predicate)
        if true_only:
            return true_filter
        else:
            return (true_filter, false_filter)
        

    def split_fields(self, fields, true_only = False):
        return self.split(lambda predicate: predicate.get_key() in fields, 
                true_only)

    def provides_key_field(self, key_fields):
        # No support for tuples
        for field in key_fields:
            if not self.has_op(field, eq) and not self.has_op(field, included):
                # Missing key fields in query filters
                return False
        return True

    def rename(self, aliases):
        for predicate in self:
            predicate.rename(aliases)
        return self

    def get_field_values(self, field):
        """
        This function returns the values that are determined by the filters for
        a given field, or None is the filter is not *setting* determined values.

        Returns: list : a list of fields
        """
        value_list = list()
        for predicate in self:
            key, op, value = predicate.get_tuple()

            if key == field:
                extract_tuple = False
            elif key == (field, ):
                extract_tuple = True
            else:
                continue

            if op == eq:
                if extract_tuple:
                    value = value[0]
                value_list.append(value)
            elif op == included:
                if extract_tuple:
                    value = [x[0] for x in value]
                value_list.extend(value)
            else:
                continue

        return list(set(value_list))

    def update_field_value_eq(self, field, value):
        for predicate in self:
            p_field, p_op, p_value = predicate.get_tuple()
            if p_field == field:
                predicate.set_op(eq)
                predicate.set_value(value)
                break # assuming there is a single predicate with field/op

    def __and__(self, other):
        # Note: we assume the predicates in self and other are already in
        # minimal form, eg. not the same fields twice...  We could break after
        # a predicate with the same key is found btw...
        s = self.copy()
        for o_predicate in other:
            o_key, o_op, o_value = o_predicate.get_tuple()

            key_found = False
            for predicate in s:
                key, op, value = predicate.get_tuple()
                if key != o_key:
                    continue

                # We already have a predicate with the same key
                key_found = True

                if op == eq:
                    if o_op == eq:
                        # Similar filters...
                        if value != o_value:
                            # ... with different values
                            return None
                        else:
                            # ... with same values
                            pass
                    elif o_op == included:
                        # Inclusion
                        if value not in o_value:
                            # no overlap
                            return None
                        else:
                            # We already have the more restrictive predicate...
                            pass
                            
                elif op == included:
                    if o_op == eq:
                        if o_value not in value:
                            return None
                        else:
                            # One value overlaps... update the initial predicate
                            # with the more restrictive one
                            predicate.set_op(eq)
                            predicate.set_value(value)
                    elif o_op == included:
                        intersection = set(o_value) & set(value)
                        if not set(o_value) & set(value):
                            return None
                        else:
                            predicate.set_value(tuple(intersection))
            
            # No conflict found, we can add the predicate to s
            if not key_found:
                s.add(o_predicate) 

        return s
