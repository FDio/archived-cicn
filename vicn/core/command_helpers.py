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

from vicn.core.commands          import Command, Commands

CMD_PRINT_TO_FILE = 'echo -n "{content}" > {filename}'

class CommandHelper:

    @staticmethod
    def if_cmd(cmd_condition, if_true = None, if_false = None):
        cmd = cmd_condition
        if if_true:
            cmd = cmd & if_true
        if if_false:
            cmd = cmd | if_false
        return cmd
            
    @staticmethod
    def file_exists(filename, if_true = None, if_false = None):
        cmd = Command('test -f {}'.format(filename))
        return CommandHelper.if_cmd(cmd, if_true, if_false)

    @staticmethod
    def print_to_file(node, filename, content):
        escaped_content = content.replace('{', '{{').replace('}', '}}')
        return BashTask(self.node, CMD_PRINT_TO_FILE, 
                {'content': escaped_content, 'filename': filename})

    @staticmethod
    def print_to_file_no_escape(filename, content):
        return BashTask(self.node, CMD_PRINT_TO_FILE, 
                {'content': content, 'filename': filename})
