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

import asyncio
import logging

from netmodel.model.type        import String
from vicn.core.attribute        import Attribute, Multiplicity
from vicn.core.exception        import ResourceNotFound
from vicn.core.requirement      import Requirement
from vicn.core.resource         import Resource
from vicn.core.task             import BashTask, EmptyTask, async_task
from vicn.core.task             import inline_task, run_task
from vicn.resource.node         import Node

log = logging.getLogger(__name__)

CMD_APT_GET_KILL = 'kill -9 $(pidof apt-get) || true'
CMD_DPKG_CONFIGURE_A = 'dpkg --configure -a'

CMD_APT_GET_UPDATE = '''
# Force IPv4
echo 'Acquire::ForceIPv4 "true";' > /etc/apt/apt.conf.d/99force-ipv4
# Update package repository on node {node}
apt-get update
'''

# We need to double { } we want to preserve
CMD_PKG_TEST='dpkg -s {self.package_name}'

CMD_PKG_INSTALL='''
# Installing package {package_name}
apt-get -y --allow-unauthenticated install {package_name}
'''

CMD_PKG_UNINSTALL='''
# Uninstalling package {self.package_name}
apt-get remove {self.package_name}
'''

CMD_SETUP_REPO = '''
# Initialize package repository {repository.repo_name} on node {self.node.name}
echo "{deb_source}" > {path}
'''

class PackageManager(Resource):
    """
    Resource: PackageManager

    APT package management wrapper.
    
    Todo:
      - We assume a package manager is always installed on every machine.
      - Currently, we limit ourselves to debian/ubuntu, and voluntarily don't
      subclass this as we have (so far) no code for selecting the right
      subclass, eg choising dynamically between DebRepositoryManager and
      RpmRepositoryManager.
      - We currently don't use package version numbers, which means a package
      can be installed but not be up to date.
    """

    node = Attribute(Node,
            reverse_name = 'package_manager',
            reverse_auto = True,
            mandatory = True,
            multiplicity = Multiplicity.OneToOne)

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._up_to_date = False
        self.apt_lock = asyncio.Lock()

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __after__(self):
        if self.node.__class__.__name__ == 'Physical':
            # UGLY : This blocking code is currently needed
            task = self.node.host_interface._get_ip4_address()
            ip_dict = task.execute_blocking()
            self.node.host_interface.ip4_address = ip_dict['ip4_address']
            return ('Repository',)
        else:
            return ('Repository', 'CentralIP', 'RoutingTable')

    @inline_task
    def __get__(self):
        raise ResourceNotFound

    def __create__(self):
        repos = EmptyTask()
        for repository in self._state.manager.by_type_str('Repository'):
            deb_source = self._get_deb_source(repository)
            path = self._get_path(repository)
            repo = BashTask(self.node, CMD_SETUP_REPO, 
                    {'deb_source': deb_source, 'path': path})
            repos = repos | repo

        return repos 

    #---------------------------------------------------------------------------
    # Methods
    #---------------------------------------------------------------------------
        
    def __method_update__(self):
        kill = BashTask(self.node, CMD_APT_GET_KILL, {'node': self.node.name},
                lock = self.apt_lock)

        # Setup during a reattempt
        if hasattr(self, '_dpkg_configure_a'):
            dpkg_configure_a = BashTask(self.node, CMD_DPKG_CONFIGURE_A,
                    lock = self.apt_lock)
        else:
            dpkg_configure_a = EmptyTask()

        if not self.node.package_manager._up_to_date:
            update = BashTask(self.node, CMD_APT_GET_UPDATE, {'node': self.node.name},
                    lock = self.apt_lock, post = self._mark_updated)
        else:
            update = EmptyTask()

        return (kill > dpkg_configure_a) > update

    def __method_install__(self, package_name):
        update = self.__method_update__()
        install = BashTask(self.node, CMD_PKG_INSTALL, {'package_name':
                package_name}, lock = self.apt_lock)
        return update > install

    #---------------------------------------------------------------------------
    # Internal methods
    #---------------------------------------------------------------------------

    def _mark_updated(self):
        self._up_to_date = True

    def _get_path(self, repository):
        return '/etc/apt/sources.list.d/{}.list'.format(repository.repo_name)

    def _get_deb_source(self, repository):
        path = repository.node.host_interface.ip4_address + '/'
        if repository.directory:
            path += repository.directory + '/'
        return 'deb http://{} {}/'.format(path, self.node.dist)

#------------------------------------------------------------------------------

class Package(Resource):
    """
    Resource: Package

    deb package support
    """

    package_name = Attribute(String, mandatory = True)
    node = Attribute(Node, 
            mandatory = True,
            requirements=[
                Requirement('package_manager')
            ])

    #---------------------------------------------------------------------------
    # Resource lifecycle
    #---------------------------------------------------------------------------

    def __get__(self):
        return BashTask(self.node, CMD_PKG_TEST, {'self': self})

    def __create__(self):
        return self.node.package_manager.__method_install__(self.package_name)

    @async_task
    async def __delete__(self):
        with await self.node.package_manager._lock:
            task = BashTask(self.node, CMD_PKG_UNINSTALL, {'self': self})
            ret = await run_task(task, self._state.manager)
            return ret

#------------------------------------------------------------------------------

class Packages(Resource):
    """
    Resource: Packages

    Todo:
     - The number of concurrent subresources is not dynamically linked to the
    nodes. We may need to link subresources to the attribute in general, but
    since package_names are static for a resource, this is not a problem here.
    """
    names = Attribute(String, multiplicity = Multiplicity.OneToMany)
    node = Attribute(Node, 
            mandatory = True,
            requirements=[
                Requirement('package_manager')
            ])

    #---------------------------------------------------------------------------
    # Resource lifecycle
    #---------------------------------------------------------------------------

    def __subresources__(self):
        """
        Note: Although packages are (rightfully) specified concurrent, apt tasks
        will be exlusive thanks to the use of a lock in the package manager.
        """
        if self.names:
            packages = [Package(node=self.node, package_name=name, owner=self)
                    for name in self.names]
            return Resource.__concurrent__(*packages)
        else:
            return None
        
