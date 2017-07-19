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

from netmodel.model.type        import String, Bool
from vicn.core.attribute        import Attribute, Multiplicity
from vicn.resource.application  import Application

class Repository(Application):
    """
    Resource: Repository

    deb package repository

    Note: As PackageManager uses a Repository, this resource cannot be a
    LinuxApplication resource. We have no package to install since they are
    part of any basic distribution install.
    """

    repo_name = Attribute(String, description = 'Name of the repository',
            default = 'vicn')
    directory = Attribute(String, description = 'Directory holding packages',
            default = '')
    sections = Attribute(String, description = 'Sections',
            multiplicity = Multiplicity.OneToMany,
            default = [])
    distributions = Attribute(String,
            description = 'List of distributions served by this repository',
            multiplicity = Multiplicity.ManyToMany,
            default = ['sid', 'trusty', 'xenial'])
    ssl = Attribute(Bool, description = 'Use SSL (https) for repository',
            default = True)
