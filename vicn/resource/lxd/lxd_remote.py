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

from vicn.core.resource         import Resource
from netmodel.model.type        import String, Bool
from vicn.core.attribute        import Attribute, Multiplicity
from vicn.core.task             import BashTask, EmptyTask, inherit_parent
from vicn.core.exception        import ResourceNotFound

# From LXD 2.14
#
# lxc remote add [<remote>] <IP|FQDN|URL> [--accept-certificate] [--password=PASSWORD] [--public] [--protocol=PROTOCOL]
# lxc remote remove <remote>
# lxc remote list
# lxc remote rename <old name> <new name>
# lxc remote set-url <remote> <url>
# lxc remote set-default <remote>
# lxc remote get-default

CMD_GET = 'lxc remote list | grep {remote.name}'

CMD_CREATE  = 'lxc remote add {remote.name} {remote.url}{public}'
CMD_CREATE += ' --protocol {remote.protocol}'

CMD_DELETE = 'lxc remote remove {remote.name}'

CMD_SET_URL = 'lxc remote set-url {remote.name} {remote.url}'

CMD_GET_DEFAULT = 'lxc remote get-default'
CMD_SET_DEFAULT = 'lxc remote set-default {remote.name}'

LxdProtocol = String.restrict(choices = ('simplestreams', 'lxd'))

class LxdRemote(Resource):
    # name (inherited)
    url = Attribute(String, mandatory = True)
    protocol = Attribute(LxdProtocol, default='lxd')
    public = Attribute(Bool, default = True)
    static = Attribute(Bool, default = False)
    default = Attribute(Bool, default = False)

    # Used to identify the LXD instance
    node = Attribute(Resource, mandatory=True)

    @inherit_parent
    def __get__(self):
        def parse(rv):
            if not rv.stdout:
                raise ResourceNotFound
            return {
                'url': self.url,
                'protocol': self.protocol,
                'public': self.public,
                'static': self.static,
                'default': self.default,
            }
        return BashTask(self.node, CMD_GET, {'remote': self}, parse = parse)

    @inherit_parent
    def __create__(self):
        public = ' --public' if self.public else ''
        return BashTask(self.node, CMD_CREATE, {'remote': self, 'public': public})

    @inherit_parent
    def __delete__(self):
        return BashTask(self.node, CMD_DELETE, {'remote': self})

    def _get_url(self):
        return None

    def _set_url(self):
        return BashTask(self.node, CMD_SET_URL, {'remote': self})

    def _get_default(self):
        def parse(rv):
            return {'default': rv.stdout == self.name}
        return BashTask(self.node, CMD_GET_DEFAULT, {'remote': self},
                parse = parse)

    def _set_default(self):
        if self.default:
            return BashTask(self.node, CMD_SET_DEFAULT, {'remote': self})
        else:
            return EmptyTask()
