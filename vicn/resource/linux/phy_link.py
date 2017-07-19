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

from vicn.core.attribute                import Attribute, Reference
from vicn.core.task                     import inline_task, async_task
from vicn.core.task                     import inherit_parent
from vicn.resource.channel              import Channel
from vicn.resource.linux.phy_interface  import PhyInterface
from vicn.resource.vpp.interface        import VPPInterface

class PhyLink(Channel):
    """
    Resource: PhyLink

    Physical Link to inform the orchestrator about Layer2 connectivity.
    """

    src = Attribute(PhyInterface, description = 'Source interface',
            mandatory = True)
    dst = Attribute(PhyInterface, description = 'Destination interface',
            mandatory = True)

    @inherit_parent
    @inline_task
    def __initialize__(self):
        self.src.set('channel', self)
        self.dst.set('channel', self)

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    async def _commit(self):
        manager = self._state.manager

        # Disable rp_filtering
        # self.src.rp_filter = False
        # self.dst.rp_filter = False

        #XXX VPP
        vpp_src = VPPInterface(parent = self.src,
                vpp = self.src.node.vpp,
                ip4_address = Reference(self.src, 'ip4_address'),
                ip6_address = Reference(self.src, 'ip6_address'),
                device_name = self.src.device_name)
        manager.commit_resource(vpp_src)


        vpp_dst = VPPInterface(parent = self.dst,
                vpp = self.dst.node.vpp,
                ip4_address = Reference(self.dst, 'ip4_address'),
                ip6_address = Reference(self.dst, 'ip6_address'),
                device_name = self.dst.device_name)
        manager.commit_resource(vpp_dst)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __get__(self):
        return  async_task(self._commit)()

    def __create__(self):
        assert self.src.node.get_type() == 'lxccontainer'
        assert self.dst.node.get_type() == 'lxccontainer'

        return async_task(self._commit)()
