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

from netmodel.model.key     import Key
from netmodel.model.type    import String, Bool, Integer
from vicn.core.attribute    import Attribute, Multiplicity
from vicn.core.exception    import ResourceNotFound
from vicn.core.resource     import Resource
from vicn.core.task         import BashTask, inline_task, EmptyTask
from vicn.resource.node     import Node

CREATE_FOLDER_CMD = "mkdir -p {folder.foldername}"
DELETE_FOLDER_CMD = "rm -f {folder.foldername}"

GET_FOLDER_CMD = 'test -d {folder.foldername} && readlink -e {folder.foldername}'

SET_FOLDER_PERMISSION_CMD = 'chmod {folder.permission} {folder.foldername}'

class Folder(Resource):
    """
    Resource: Folder
    """
    foldername = Attribute(String, description = 'Path to the folder',
            mandatory = True)
    node = Attribute(Node, description = 'Node on which the directory is created',
            mandatory = True,
            multiplicity = Multiplicity.ManyToOne,
            reverse_name = 'folders',
            reverse_description = 'Folders created on the node')
    overwrite = Attribute(Bool,
            description = 'Determines whether an existing folder is overwritten',
            default = False)
    permission = Attribute(Integer,
            description = 'Permission to set in the folder',
            default = 775)

    __key__ = Key(node, foldername)
    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    @inline_task
    def __get__(self):
        # UGLY
#        @inline_task
#        def not_found():
        raise ResourceNotFound

        # if self.overwrite:
        #     return not_found()

        # def is_path (rv):
        #     if rv is None or rv.stdout is None or len(rv.stdout) == 0 or \
        #             rv.return_value != 0:
        #         raise ResourceNotFound
        #     return {} # 'filename': rv.stdout}

        # create = BashTask(self.node, GET_FOLDER_CMD, {"folder": self}, parse=is_path)

        # return create

    def __create__(self):
        ctask = BashTask(self.node, CREATE_FOLDER_CMD, {"folder": self})

        if self.overwrite:
            ctask = BashTask(self.node, DELETE_FOLDER_CMD, {'folder': self}) > ctask

        set_permission =  BashTask(self.node, SET_FOLDER_PERMISSION_CMD, {"folder": self})

        return ctask > set_permission

    def __delete__(self):
        return BashTask(self.node, DELETE_FOLFER_CMD, { "folder" : self})
