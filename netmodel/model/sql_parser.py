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

import re
import sys
import pyparsing as pp

from netmodel.model.query           import Query
from netmodel.model.filter          import Filter
from netmodel.model.predicate       import Predicate

DEBUG = False

def debug(args):
    if DEBUG: print(args)

class SQLParser(object):

    def __init__(self):
        """
        Our simple BNF:
        SELECT [fields[*] FROM table WHERE clause
        """

        integer = pp.Combine(pp.Optional(pp.oneOf("+ -")) + 
                pp.Word(pp.nums)).setParseAction(lambda t:int(t[0]))
        floatNumber = pp.Regex(r'\d+(\.\d*)?([eE]\d+)?')
        point = pp.Literal(".")
        e     = pp.CaselessLiteral("E")

        kw_store   = pp.CaselessKeyword('=')
        kw_select  = pp.CaselessKeyword('select')
        kw_subscribe = pp.CaselessKeyword('subscribe')
        kw_update  = pp.CaselessKeyword('update')
        kw_insert  = pp.CaselessKeyword('insert')
        kw_delete  = pp.CaselessKeyword('delete')
        kw_execute = pp.CaselessKeyword('execute')

        kw_from    = pp.CaselessKeyword('from')
        kw_into    = pp.CaselessKeyword('into')
        kw_where   = pp.CaselessKeyword('where')
        kw_at      = pp.CaselessKeyword('at')
        kw_set     = pp.CaselessKeyword('set')
        kw_true    = pp.CaselessKeyword('true').setParseAction(lambda t: 1)
        kw_false   = pp.CaselessKeyword('false').setParseAction(lambda t: 0)
        kw_with    = pp.CaselessKeyword('with')

        sum_function     = pp.CaselessLiteral('sum')

        # Regex string representing the set of possible operators
        # Example : ">=|<=|!=|>|<|="
        OPERATOR_RX = "(?i)%s" % '|'.join([re.sub('\|', '\|', o) \
                for o in Predicate.operators.keys()])
        
        # predicate
        field      = pp.Word(pp.alphanums + '_' + '.')
        operator   = pp.Regex(OPERATOR_RX).setName("operator")
        variable   = pp.Literal('$').suppress() + pp.Word(pp.alphanums \
                + '_' + '.').setParseAction(lambda t: "$%s" % t[0])
        filename   = pp.Regex('([a-z]+?://)?(?:[a-zA-Z]|[0-9]|[$-_@.&+]|[!*\(\),]|(?:%[0-9a-fA-F][0-9a-fA-F]))+')

        obj        = pp.Forward()
        value      = obj | pp.QuotedString('"') | pp.QuotedString("'") | \
                kw_true | kw_false | integer | variable
        
        def handle_value_list(s, l, t):
            t = t.asList()
            new_t = tuple(t)
            debug("[handle_value_list] s = %(s)s ** l = %(l)s ** t = %(t)s" \
                    % locals())
            debug("                    new_t = %(new_t)s" % locals())
            return new_t

        value_list = value \
               | (pp.Literal("[").suppress() + pp.Literal("]").suppress()) \
                    .setParseAction(lambda s, l, t: [[]]) \
               | pp.Literal("[").suppress() \
                    + pp.delimitedList(value).setParseAction(handle_value_list) \
                    + pp.Literal("]") \
                    .suppress()

        table      = pp.Word(pp.alphanums + ':_-/').setResultsName('object_name')
        field_list = pp.Literal("*") | pp.delimitedList(field).setParseAction(lambda tokens: set(tokens))

        assoc      = (field + pp.Literal(":").suppress() + value_list).setParseAction(lambda tokens: [tokens.asList()])
        obj        << pp.Literal("{").suppress() \
                    + pp.delimitedList(assoc).setParseAction(lambda t: dict(t.asList())) \
                    + pp.Literal("}").suppress()

        # PARAMETER (SET)
        # X = Y    -->    t=(X, Y)
        def handle_param(s, l, t):
            t = t.asList()
            assert len(t) == 2
            new_t = tuple(t)
            debug("[handle_param] s = %(s)s ** l = %(l)s ** t = %(t)s" % locals())
            debug("               new_t = %(new_t)s" % locals())
            return new_t

        param      = (field + pp.Literal("=").suppress() + value_list) \
            .setParseAction(handle_param)

        # PARAMETERS (SET)
        # PARAMETER[, PARAMETER[, ...]]    -->    dict()
        def handle_parameters(s, l, t):
            t = t.asList()
            new_t = dict(t) if t else dict()
            debug("[handle_parameters] s = %(s)s ** l = %(l)s ** t = %(t)s" % locals())
            debug("                    new_t = %(new_t)s" % locals())
            return new_t

        parameters = pp.delimitedList(param) \
            .setParseAction(handle_parameters)

        predicate  = (field + operator + value_list).setParseAction(self.handlePredicate)

        # For the time being, we only support simple filters and not full clauses
        filter     = pp.delimitedList(predicate, delim='&&').setParseAction(lambda tokens: Filter(tokens.asList()))

        datetime   = pp.Regex(r'....-..-.. ..:..:..')

        timestamp  = pp.CaselessKeyword('now') | datetime

        store_elt  = (variable.setResultsName("variable") + kw_store.suppress())
        fields_elt = field_list.setResultsName('field_names')
        aggregate_elt = sum_function.setResultsName('aggregate') + pp.Literal("(").suppress() + fields_elt + pp.Literal(")").suppress()
        select_elt = (kw_select.suppress() + fields_elt)
        subscribe_elt = (kw_subscribe.suppress() + fields_elt)
        where_elt  = (kw_where.suppress()  + filter.setResultsName('filter'))
        set_elt    = (kw_set.suppress()    + parameters.setResultsName('params'))
        at_elt     = (kw_at.suppress()     + timestamp.setResultsName('timestamp'))
        into_elt   = (kw_into.suppress()   + filename.setResultsName('receiver'))

        # SELECT [SUM(]*|field_list[)] [AT timestamp] FROM table [WHERE clause]
        select     = (
              pp.Optional(store_elt)\
            + kw_select.suppress() \
            + pp.Optional(into_elt) \
            + (aggregate_elt | fields_elt)\
            + pp.Optional(at_elt)\
            + kw_from.suppress()\
            + table\
            + pp.Optional(where_elt)
        ).setParseAction(lambda args: self.action(args, 'select'))

        subscribe     = (
              pp.Optional(store_elt) \
            + kw_subscribe.suppress() \
            + pp.Optional(into_elt) \
            + (aggregate_elt | fields_elt) \
            + pp.Optional(at_elt)\
            + kw_from.suppress()\
            + table\
            + pp.Optional(where_elt)
            + pp.Optional(set_elt)
        ).setParseAction(lambda args: self.action(args, 'subscribe'))

        # UPDATE table SET parameters [WHERE clause] [SELECT *|field_list]
        update     = (
              kw_update \
            + table \
            + set_elt \
            + pp.Optional(where_elt) \
            + pp.Optional(select_elt) 
        ).setParseAction(lambda args: self.action(args, 'update'))

        # INSERT INTO table SET parameters  [SELECT *|field_list]
        insert     = (
                kw_insert + kw_into + table
              + set_elt
              + pp.Optional(select_elt)
        ).setParseAction(lambda args: self.action(args, 'insert'))

        # DELETE FROM table [WHERE clause]
        delete     = (
              kw_delete \
            + kw_from \
            + table \
            + pp.Optional(where_elt)
        ).setParseAction(lambda args: self.action(args, 'delete'))

        # 
        execute    = (
                kw_execute + kw_from + table
              + set_elt
              + pp.Optional(where_elt)
        ).setParseAction(lambda args: self.action(args, 'execute'))

        annotation = pp.Optional(kw_with + \
                parameters.setResultsName('annotation'))

        self.bnf = (select | update | insert | delete | subscribe | execute) \
                   + annotation

        # For reusing parser:
        self.filter = filter

    def action(self, args, action):
        args['action'] = action

    def handlePredicate(self, args):
        return Predicate(*args)

    def parse(self, string):
        result = self.bnf.parseString(string, parseAll = True)
        return dict(result.items())
