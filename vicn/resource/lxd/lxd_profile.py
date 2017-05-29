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

from vicn.core.resource                 import Resource
from netmodel.model.type                import String
from vicn.core.attribute                import Attribute, Multiplicity
from vicn.core.task                     import BashTask
from vicn.core.exception              import ResourceNotFound

CMD_LXD_PROFILE_CREATE = '''
lxc profile create {profile.name} description="{profile.description}"
lxc profile device add {profile.name} root disk pool={profile.pool} path=/
lxc profile device add {profile.name} {profile.iface_name} nic name={profile.iface_name} nictype=bridged parent={profile.network}
lxc profile unset {profile.name} environment.http_proxy
lxc profile unset {profile.name} user.network_mode
# Temp fix for VPP
lxc profile create vpp
'''

CMD_LXD_PROFILE_GET = 'lxc profile list | grep {profile.name}'

# Default name of VICN management/monitoring interface
#
# This should be kept in sync with /etc/network/interfaces in the image file so that dhcp works
LXD_PROFILE_DEFAULT_IFNAME = 'vicn_mgmt'

class LxdProfile(Resource):

    description = Attribute(String, descr="profile description", mandatory=True)
    pool = Attribute(String, descr="ZFS pool used by the containers", mandatory=True)
    network = Attribute(String, description='Network on which to attach', mandatory=True)
    iface_name = Attribute(String, description='Default interface name',
            default = LXD_PROFILE_DEFAULT_IFNAME)
    node = Attribute(Resource, mandatory=True)

    def __get__(self):
        def parse(rv):
            if not rv.stdout:
                raise ResourceNotFound
        return BashTask(self.node, CMD_LXD_PROFILE_GET, {'profile':self}, parse=parse)

    def __create__(self):
        return BashTask(self.node, CMD_LXD_PROFILE_CREATE, {'profile':self})
