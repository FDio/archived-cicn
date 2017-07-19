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

from netmodel.model.filter          import Filter
from netmodel.model.field_names     import FieldNames

ACTION_INSERT    = 1
ACTION_SELECT    = 2
ACTION_UPDATE    = 3
ACTION_DELETE    = 4
ACTION_EXECUTE   = 5
ACTION_SUBSCRIBE = 6
ACTION_UNSUBSCRIBE = 7

ACTION2STR = {
    ACTION_INSERT    : 'insert',
    ACTION_SELECT    : 'select',
    ACTION_UPDATE    : 'update',
    ACTION_DELETE    : 'delete',
    ACTION_EXECUTE   : 'execute',
    ACTION_SUBSCRIBE : 'subscribe',
    ACTION_UNSUBSCRIBE : 'unsubscribe',
}
STR2ACTION = dict((v, k) for k, v in ACTION2STR.items())

FUNCTION_SUM      = 1

FUNCTION2STR = {
    FUNCTION_SUM     : 'sum'
}
STR2FUNCTION = dict((v, k) for k, v in FUNCTION2STR.items())

class Query:
    def __init__(self, action, object_name, filter = None, params = None,
            field_names = None, aggregate = None, last = False, reply = False):
        self.action      = action
        self.object_name = object_name

        if filter:
            if isinstance(filter, Filter):
                self.filter = filter
            else:
                self.filter = Filter.from_list(filter)
        else:
            self.filter = Filter()

        self.params = params

        if field_names:
            if isinstance(field_names, FieldNames):
                self.field_names = field_names
            else:
                self.field_names = FieldNames(field_names)
        else:
            self.field_names = FieldNames()

        self.aggregate = aggregate

        self.last = last
        self.reply = reply

    def to_dict(self):
        aggregate = FUNCTION2STR[self.aggregate] if self.aggregate else None
        return {
            'action': ACTION2STR[self.action],
            'object_name': self.object_name,
            'filter': self.filter.to_list(),
            'params': self.params,
            'field_names': self.field_names,
            'aggregate': aggregate,
            'reply': self.reply,
            'last': self.last
        }

    @staticmethod
    def from_dict(dic):
        action = STR2ACTION[dic.get('action').lower()]
        object_name = dic.get('object_name')
        filter = dic.get('filter', None)
        params = dic.get('params', None)
        field_names = dic.get('field_names', None)
        aggregate = STR2FUNCTION[dic.get('aggregate').lower()] \
                    if dic.get('aggregate') else None
        if field_names  == '*':
            field_names = FieldNames(star = True)
        last = dic.get('last', False)
        reply = dic.get('reply', False)
        return Query(action, object_name, filter, params, field_names,
                aggregate, last)

    def to_sql(self, multiline = False):
        """
        Args:
            platform: A String corresponding to a namespace (or platform name)
            multiline: A boolean indicating whether the String could contain
                carriage return.
        Returns:
            The String representing this Query.
        """
        get_params_str = lambda : ", ".join(["%s = %r" % (k, v) \
                for k, v in self.params.items()])

        object_name  = self.object_name
        field_names = self.field_names
        field_names_str = ('*' if field_names.is_star() \
                else ', '.join([field for field in field_names]))
        select = "SELECT %s" % ((FUNCTION2STR[self.aggregate] + "(%s)") \
                if self.aggregate else '%s') % field_names_str
        filter  = "WHERE %s" % self.filter      if self.filter           else ''
        #at     = "AT %s"     % self.get_timestamp() if self.get_timestamp() else ""
        at = ''
        params = "SET %s"    % get_params_str()     if self.params       else ''

        sep = " " if not multiline else "\n  "

        strmap = {
            ACTION_SELECT : "%(select)s%(sep)s%(at)s%(sep)sFROM %(object_name)s%(sep)s%(filter)s",
            ACTION_UPDATE : "UPDATE %(object_name)s%(sep)s%(params)s%(sep)s%(filter)s%(sep)s%(select)s",
            ACTION_INSERT : "INSERT INTO %(object_name)s%(sep)s%(params)s",
            ACTION_DELETE : "DELETE FROM %(object_name)s%(sep)s%(filter)s",
            ACTION_SUBSCRIBE : "SUBSCRIBE : %(select)s%(sep)s%(at)s%(sep)sFROM %(object_name)s%(sep)s%(filter)s",
            ACTION_UNSUBSCRIBE : "UNSUBSCRIBE : %(select)s%(sep)s%(at)s%(sep)sFROM %(object_name)s%(sep)s%(filter)s",
            ACTION_EXECUTE : "EXECUTE : %(select)s%(sep)s%(at)s%(sep)sFROM %(object_name)s%(sep)s%(filter)s",
        }

        return strmap[self.action] % locals()

    def __repr__(self):
        return self.to_sql()

    __str__ = __repr__
