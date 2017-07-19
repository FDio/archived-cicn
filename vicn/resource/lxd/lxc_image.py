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

import logging
import time

from netmodel.model.type                import Self
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.exception                import ResourceNotFound
from vicn.core.requirement              import Requirement
from vicn.core.resource                 import Resource
from vicn.core.task                     import task, inline_task, inherit_parent
from vicn.resource.linux.application    import LinuxApplication as Application
from vicn.resource.node                 import Node

log = logging.getLogger(__name__)

class LxcImage(Resource):
    """
    Resource: LxcImage
    """

    node = Attribute(Node, description = 'Node on which the image is stored',
            mandatory = True,
            requirements = [
                Requirement('lxd_hypervisor')
            ])
    image = Attribute(Self, description = 'image', default = None)
    applications = Attribute(Application, multiplicity = Multiplicity.OneToMany)

    #---------------------------------------------------------------------------
    # Constructor / Accessors
    #---------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        self.fingerprint = None
        self._tmp_container = None
        super().__init__(*args, **kwargs)

    #---------------------------------------------------------------------------
    # Resource lifecycle
    #---------------------------------------------------------------------------

    @inherit_parent
    @task
    def __get__(self):
        log.warning('Image test is currently disabled')
        return
        aliases = [alias['name'] for images in self.node.lxd_hypervisor.client.images.all()
                         for alias in images.aliases]
        if not self.image in aliases:
            raise ResourceNotFound

    @inline_task
    def __create__(self):
        log.warning('Image creation is currently disabled')
        return


    @inherit_parent
    @task
    def __create_DISABLED__(self):
        """
        Image creation consists in setting up a temporary container, stopping
        it, publishing an image of it, setting an alias, and deleting it.
        """
        tmp_container.setup()

        print("TODO: Installing applications...")
        for application in self.applications:
            print('Installing application on image')
            application.setup()

        # XXX stop() hangs if run to early wrt container start
        # - is it related to ZFS ? is it a more general problem ?
        time.sleep(5)

        print("I: Stopping container")
        tmp_container.stop()

        print("I: Publishing image")
        image_metadata = tmp_container.publish_image() # METHOD !
        print("MD=", image_metadata)
        self.fingerprint = image_metadata['fingerprint']
        self.set_alias()

        tmp_container.delete()

    @inherit_parent
    @task
    def __delete__(self):
        self.node.lxd_hypervisor.client.images.delete(self.name)

    #---------------------------------------------------------------------------
    # Public methods
    #---------------------------------------------------------------------------

    def set_alias(self):
        alias_dict = {
            "description": "Ubuntu 16.04 image with ICN software already installed",
            "target": self.fingerprint,
            "name": self.name
        }
        self.node.lxd_hypervisor.set_alias(alias_dict)
