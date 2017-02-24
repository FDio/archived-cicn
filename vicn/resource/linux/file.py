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

from netmodel.model.type    import String, Bool
from vicn.core.attribute    import Attribute, Multiplicity
from vicn.core.exception    import ResourceNotFound
from vicn.core.resource     import Resource
from vicn.core.task         import BashTask, inline_task
from vicn.resource.node     import Node

CREATE_DIR_CMD  = "mkdir -p {dir}"
CREATE_FILE_CMD = "mkdir -p $(dirname {file.filename}) && touch {file.filename}"
DELETE_FILE_CMD = "rm -f {file.filename}"

GET_FILE_CMD = 'test -f {file.filename} && readlink -e {file.filename}'

GREP_FILE_CMD   = "cat {file.filename}"

CMD_PRINT_TO_FILE = 'echo -n "{file.content}" > {file.filename}'

class File(Resource):
    """
    Resource: File
    """
    filename = Attribute(String, description = 'Path to the file', 
            mandatory = True)
    node = Attribute(Node, description = 'Node on which the file is created',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne,
            reverse_name = 'files',
            reverse_description = 'Files created on the node')
    overwrite = Attribute(Bool, 
            description = 'Determines whether an existing file is overwritten',
            default = False)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __get__(self):

        # UGLY
        @inline_task
        def not_found():
            raise ResourceNotFound

        if self.overwrite: 
            return not_found()

        def is_path (rv):
            if rv is None or rv.stdout is None or len(rv.stdout) == 0 or \
                    rv.return_value != 0:
                raise ResourceNotFound
            return {} # 'filename': rv.stdout}

        test = BashTask(self.node, GET_FILE_CMD, {"file": self}, parse=is_path)
        return test

    def __create__(self):
        ctask = BashTask(self.node, CREATE_FILE_CMD, {"file": self})
        if self.overwrite:
            ctask = BashTask(self.node, DELETE_FILE_CMD, {'file': self}) > ctask
        return ctask

    def __delete__(self):
        return BashTask(self.node, DELETE_FILE_CMD, { "file" : self})

#------------------------------------------------------------------------------

class TextFile(File):
    """
    Resource: TextFile

    A file with text content.
    """

    content = Attribute(String, default='')

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __create__(self):
        return BashTask(self.node, CMD_PRINT_TO_FILE, {'file': self})

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def _set_content(self):
        return self.__create__()

    def _get_content(self):
        return BashTask(self.node, GREP_FILE_CMD, {'file': self},
                parse =( lambda x : x.stdout))
