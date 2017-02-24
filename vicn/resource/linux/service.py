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

from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import CategoryResource
from vicn.core.task                     import inline_task, BashTask, EmptyTask
from vicn.resource.linux.application    import LinuxApplication

log = logging.getLogger(__name__)

CMD_START = 'service {service_name} start'
CMD_STOP = 'service {service_name} stop'
CMD_RESTART = 'service {service_name} restart'
CMD_STOP_START = 'service {service_name} stop && sleep 1 && ' \
                 'service {service_name} start'

class Service(LinuxApplication):
    """Service resource

    This resource wraps a Linux Service, and ensure the service is started
    (resp. stopped) during setup (resp. teardown).

    Required tags:
        __service_name__ (str): all classes that inherit from Service should
            inform this tag which gives the name of the service known to the
            system.

    TODO:
     * Support for upstart, sysvinit and systemd services.
     * Start and Stop method
     * Status attribute
    """

    __type__ = CategoryResource



    @inline_task
    def __get__(self):
        raise ResourceNotFound
    
    def __method_restart__(self):
        return BashTask(self.node, CMD_RESTART, 
                        {'service_name': self.__service_name__})
    
    def __method_start__(self):
        return BashTask(self.node, CMD_START,
                        {'service_name': self.__service_name__})
    

    def __create__(self):
        if self.__service_name__ == 'lxd':
            log.warning('Not restarting LXD')
            return EmptyTask()

        if self.__service_name__ == 'dnsmasq':
            return BashTask(self.node, CMD_STOP_START, 
                    {'service_name': self.__service_name__})

        return self.__method_restart__()


    def __delete__(self):
        return BashTask(self.node, CMD_STOP, 
                {'service_name': self.__service_name__})

