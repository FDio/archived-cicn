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

import ipaddress
import logging

from string                         import Template

from netmodel.model.type            import String, Bool
from vicn.core.attribute            import Attribute
from vicn.core.resource             import EmptyResource
from vicn.resource.dns_server       import DnsServer
from vicn.resource.interface        import Interface
from vicn.resource.linux.file       import TextFile
from vicn.resource.linux.service    import Service

log = logging.getLogger(__name__)

FN_CONF='/etc/dnsmasq.conf'

TPL_CONF='''
# Configuration file for dnsmasq.
#
# Format is one option per line, legal options are the same
# as the long options legal on the command line. See
# "/usr/sbin/dnsmasq --help" or "man 8 dnsmasq" for details.

interface=$interface
dhcp-range=$dhcp_range

#server=$server
$flags
'''

DHCP_OFFSET = 195

class DnsMasq(Service, DnsServer):
    """

    Todo:
     - Currently, a single interface is supported.
     - DHCP range is hardcoded
    """
    __package_names__ = ['dnsmasq']
    __service_name__ = 'dnsmasq'

    interface = Attribute(Interface,
            description = 'Interface on which to listen')
    lease_interval = Attribute(String,
            default = '12h')
    server = Attribute(String)
    dhcp_authoritative = Attribute(Bool,
            description = 'Flag: DHCP authoritative',
            default = True)
    log_queries = Attribute(Bool, description = 'Flag: log DNS queries',
            default = True)
    log_dhcp = Attribute(Bool, description = 'Flag: log DHCP queries',
            default = True)

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if not self.interface:
            raise Exception("Cannot initialize bridge without interface")

    def __subresources__(self):
        # Overwrite configuration file
        flags = list()
        if self.dhcp_authoritative:
            flags.append('dhcp-authoritative')
        if self.log_queries:
            flags.append('log-queries')
        if self.log_dhcp:
            flags.append('log-dhcp')
            network = self._state.manager.get('network')
            network = ipaddress.ip_network(network, strict=False)

            dhcp_range = '{},{},{},{},{}'.format(
                    self.interface.device_name,
                    str(network[DHCP_OFFSET]),
                    str(network[DHCP_OFFSET + 5]), # eg. .253
                    "255.255.255.0",
                    self.lease_interval)

            t_dict = {
                'interface' : self.interface.device_name,
                'dhcp_range': dhcp_range,
                'server'    : str(network[-2]), # unused so far
                'flags'     : '\n'.join(flags)
            }

        t = Template(TPL_CONF)
        conf = t.substitute(t_dict)

        return TextFile(node = self.node, owner = self, filename = FN_CONF,
                content = conf, overwrite = True)
