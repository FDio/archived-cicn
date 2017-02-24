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

from netmodel.model.type            import String, Integer
from vicn.core.attribute            import Attribute
from vicn.resource.icn.producer     import Producer

DEFAULT_CHUNK_SIZE = 1300

class VirtualRepo(Producer):
    """
    Resource: VirtualRepo

    Note:
      ndn-virtual-repo {self.folder} -s {self.chunk_size}
    """

    __package_names__ = ['ndn-virtual-repo']

    folder = Attribute(String, description = "Folder")
    chunk_size = Attribute(Integer, description = "Chunk size",
            default = DEFAULT_CHUNK_SIZE)
