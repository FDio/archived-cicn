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

import pprint
import time
import traceback

from netmodel.network.packet       import ErrorPacket
from netmodel.model.query          import Query as Record

# type
SUCCESS     = 0
WARNING     = 1
ERROR       = 2

# origin
CORE        = 0
GATEWAY     = 1

class ResultValue(dict):

    ALLOWED_FIELDS = set(["origin", "type", "code", "value", "description", 
            "traceback", "ts"])

    def __init__(self, *args, **kwargs):
        if args:
            if kwargs:
                raise Exception("Bad initialization for ResultValue")

            if len(args) == 1 and isinstance(args[0], dict):
                kwargs = args[0]

        given = set(kwargs.keys())
        cstr_success = set(["code", "origin", "value"]) <= given
        cstr_error   = set(["code", "type", "origin", "description"]) <= given
        assert given <= self.ALLOWED_FIELDS, \
                "Wrong fields in ResultValue constructor: %r" % \
                    (given - self.ALLOWED_FIELDS)
        assert cstr_success or cstr_error, \
            "Incomplete set of fields in ResultValue constructor: %r" % given

        dict.__init__(self, **kwargs)

        # Set missing fields to None
        for field in self.ALLOWED_FIELDS - given:
            self[field] = None
        if not "ts" in self:
            self["ts"] = time.time()

    def get_code(self):
        """
        Returns:
            The code transported in this ResultValue instance/
        """
        return self["code"]

    @classmethod
    def get(self, records, errors):
        num_errors = len(errors)

        if num_errors == 0:
            return ResultValue.success(records)
        elif records:
            return ResultValue.warning(records, errors)
        else:
            return ResultValue.errors(errors)

    @classmethod
    def success(self, result):
        return ResultValue(
            code        = SUCCESS,
            type        = SUCCESS,
            origin      = [CORE, 0],
            value       = result
        )

    @staticmethod
    def warning(result, errors):
        return ResultValue(
            code        = ERROR, 
            type        = WARNING,
            origin      = [CORE, 0],
            value       = result,
            description = errors
        )

    @staticmethod
    def error(description, code = ERROR):
        assert isinstance(description, str),\
            "Invalid description = %s (%s)" % (description, type(description))
        assert isinstance(code, int),\
            "Invalid code = %s (%s)" % (code, type(code))

        return ResultValue(
            type        = ERROR,
            code        = code,
            origin      = [CORE, 0],
            description = [ErrorPacket(type = ERROR, code = code, 
                message = description, traceback = None)]
        )

    @staticmethod
    def errors(errors):
        """
        Make a ResultValue corresponding to an error and
        gathering a set of ErrorPacket instances.
        Args:
            errors: A list of ErrorPacket instances.
        Returns:
            The corresponding ResultValue instance.
        """
        assert isinstance(errors, list),\
            "Invalid errors = %s (%s)" % (errors, type(errors))

        return ResultValue(
            type        = ERROR,
            code        = ERROR,
            origin      = [CORE, 0],
            description = errors
        )

    def is_warning(self):
        return self["type"] == WARNING

    def is_success(self):
        return self["type"] == SUCCESS and self["code"] == SUCCESS

    def get_all(self):
        """
        Retrieve the Records embedded in this ResultValue.
        Raises:
            RuntimeError: in case of failure.
        Returns:
            A Records instance.
        """
        if not self.is_success() and not self.is_warning():
            raise RuntimeError("Error executing query: %s" % \
                    (self["description"]))
        try:
            records = self["value"]
            if len(records) > 0 and not isinstance(records[0], Record):
                raise TypeError("Please put Record instances in ResultValue")
            return records
        except AttributeError as e:
            raise RuntimeError(e)

    def get_one(self):
        """
        Retrieve the only Record embeded in this ResultValue.
        Raises:
            RuntimeError: if there is 0 or more that 1 Record in
                this ResultValue.
        Returns:
            A list of Records (and not of dict).
        """
        records = self.get_all()
        num_records = len(records)
        if num_records != 1:
            raise RuntimeError('Cannot call get_one() with multiple records')
        return records.get_one()

    def get_error_message(self):
        return "%r" % self["description"]

    @staticmethod
    def to_html(raw_dict):
        return pprint.pformat(raw_dict).replace("\\n","<br/>")

    def to_dict(self):
        return dict(self)
