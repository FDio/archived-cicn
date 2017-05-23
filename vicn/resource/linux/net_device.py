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
import re
import math
import random
import string

from netmodel.model.type                import Integer, String, Bool
from vicn.core.address_mgr              import AddressManager
from vicn.core.attribute                import Attribute
from vicn.core.exception                import ResourceNotFound
from vicn.core.resource                 import BaseResource
from vicn.core.task                     import BashTask, task, EmptyTask
from vicn.resource.linux.application    import LinuxApplication as Application
from vicn.resource.interface            import Interface

# parse_ip_addr inspired from:
# From: https://github.com/ohmu/poni/blob/master/poni/cloud_libvirt.py

LXD_FIX = lambda cmd: 'sleep 1 && {}'.format(cmd)

MAX_DEVICE_NAME_SIZE = 15

IPV4=4
IPV6=6

CMD_FLUSH_IP         = 'ip -{ip_version} addr flush dev {device_name}'

CMD_INTERFACE_LIST = 'ip link show | grep -A 1 @{}'
RX_INTERFACE_LIST  = '.*?(?P<ifname>[^ ]*)@{}:'
CMD_INTERFACE_GET  = 'ip link show | grep -A 1 {}@{}'
RX_INTERFACE_GET   = '.*?(?P<ifname>{})@{}:'

log = logging.getLogger(__name__)

CMD_GET = LXD_FIX('ip link show {netdevice.device_name}')
CMD_CREATE = 'ip link add name {netdevice.device_name} ' \
             'type {netdevice.netdevice_type}'
CMD_CREATE_PARENT = 'ip link add name {netdevice.device_name} ' \
                    'link {netdevice.parent.device_name} ' \
                    'type {netdevice.netdevice_type}'
CMD_DELETE = 'ip link delete {netdevice.device_name}'
CMD_SET_MAC_ADDRESS = 'ip link set dev {netdevice.device_name} ' \
                      'address {netdevice.mac_address}'
CMD_GET_IP_ADDRESS = 'ip addr show {netdevice.device_name}'
CMD_SET_IP4_ADDRESS = 'ip addr add dev {netdevice.device_name} ' \
                     '{netdevice.ip4_address} brd + || true'
CMD_SET_IP6_ADDRESS = 'ip addr add dev {netdevice.device_name} ' \
                     '{netdevice.ip6_address}/{netdevice.ip6_prefix} || true'
CMD_SET_PROMISC = 'ip link set dev {netdevice.device_name} promisc {on_off}'
CMD_SET_UP = 'ip link set {netdevice.device_name} {up_down}'
CMD_SET_CAPACITY='\n'.join([
    'tc qdisc del dev {netdevice.device_name} root || true',
    'tc qdisc add dev {netdevice.device_name} root handle 1: tbf rate '
        '{netdevice.capacity}Mbit burst {burst}kb latency 70ms',
    'tc qdisc add dev {netdevice.device_name} parent 1:1 codel',
])
CMD_GET_PCI_ADDRESS='ethtool -i {netdevice.device_name} | ' \
    "sed -n '/bus-info/{{s/.*: [^:]*:\(.*\)/\\1/p}}'"
CMD_GET_OFFLOAD='ethtool -k {netdevice.device_name} | ' \
    'grep rx-checksumming | cut -d " " -f 2'
CMD_SET_OFFLOAD='ethtool -K {netdevice.device_name} rx on tx on'
CMD_UNSET_OFFLOAD='ethtool -K {netdevice.device_name} rx off tx off'

CMD_UNSET_RP_FILTER = '''
sysctl -w net.ipv4.conf.all.rp_filter=0
sysctl -w net.ipv4.conf.{netdevice.device_name}.rp_filter=0
'''
CMD_SET_RP_FILTER = 'sysctl -w ' \
    'net.ipv4.conf.{netdevice.device_name}.rp_filter=1'
CMD_GET_RP_FILTER = '''
sysctl net.ipv4.conf.all.rp_filter
sysctl net.ipv4.conf.{netdevice.device_name}.rp_filter
'''

CMD_UNSET_IP6_FWD = 'sysctl -w net.ipv6.conf.{netdevice.device_name}.forwarding=0'
CMD_SET_IP6_FWD = 'sysctl -w net.ipv6.conf.{netdevice.device_name}.forwarding=1'
CMD_GET_IP6_FWD = 'sysctl -n net.ipv6.conf.{netdevice.device_name}.forwarding'


#-------------------------------------------------------------------------------

# FIXME GPL code

# Copyright 2015 Canonical Ltd.  This software is licensed under the
# GNU Affero General Public License version 3 (see the file LICENSE).

"""Utility to parse 'ip link [show]'.

Example dictionary returned by parse_ip_link():

{u'eth0': {u'flags': set([u'BROADCAST', u'LOWER_UP', u'MULTICAST', u'UP']),
           u'index': 2,
           u'mac': u'80:fa:5c:0d:43:5e',
           u'name': u'eth0',
           u'settings': {u'group': u'default',
                         u'mode': u'DEFAULT',
                         u'mtu': u'1500',
                         u'qdisc': u'pfifo_fast',
                         u'qlen': u'1000',
                         u'state': u'UP'}},
 u'lo': {u'flags': set([u'LOOPBACK', u'LOWER_UP', u'UP']),
         u'index': 1,
         u'name': u'lo',
         u'settings': {u'group': u'default',
                       u'mode': u'DEFAULT',
                       u'mtu': u'65536',
                       u'qdisc': u'noqueue',
                       u'state': u'UNKNOWN'}}}

The dictionary above is generated given the following input:

        1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN \
mode DEFAULT group default
            link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
        2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast \
state UP mode DEFAULT group default qlen 1000
            link/ether 80:fa:5c:0d:43:5e brd ff:ff:ff:ff:ff:ff
"""

def _get_settings_dict(settings_line):
    """
    Given a string of the format:
        "[[<key1> <value1>] <key2> <value2>][...]"
    Returns a dictionary mapping each key to its corresponding value.
    :param settings_line: unicode
    :return: dict
    """
    settings = settings_line.strip().split()
    num_tokens = len(settings)
    assert num_tokens % 2 == 0
    return {
        settings[2 * i]: settings[2 * i + 1] for i in range(num_tokens // 2)
        }


def _parse_interface_definition(line):
    """Given a string of the format:
        <interface_index>: <interface_name>: <flags> <settings>
    Returns a dictionary containing the component parts.
    :param line: unicode
    :return: dict
    :raises: ValueError if a malformed interface definition line is presented
    """
    interface = {}

    # This line is in the format:
    # <interface_index>: <interface_name>: <properties>
    [index, name, properties] = map(
        lambda s: s.strip(), line.split(':'))

    interface['index'] = int(index)
    if '@' in name:
        name, parent = name.split('@')
        interface['name'] = name
        interface['parent'] = parent
    else:
        interface['name'] = name
        interface['parent'] = None

    # Now parse the <properties> part from above.
    # This will be in the form "<FLAG1,FLAG2> key1 value1 key2 value2 ..."
    matches = re.match(r"^<(.*)>(.*)", properties)
    if matches:
        flags = matches.group(1)
        if len(flags) > 0:
            flags = flags.split(',')
        else:
            flags = []
        interface['flags'] = set(flags)
        interface['settings'] = _get_settings_dict(matches.group(2))
    else:
        raise ValueError("Malformed 'ip link' line (%s)" % line)
    return interface


def _add_additional_interface_properties(interface, line):
    """
    Given the specified interface and a specified follow-on line containing
    more interface settings, adds any additional settings to the interface
    dictionary. (currently, the only relevant setting is the interface MAC.)
    :param interface: dict
    :param line: unicode
    """
    settings = _get_settings_dict(line)
    # We only care about the MAC address for Ethernet interfaces.
    mac = settings.get('link/ether')
    if mac is not None:
        interface['mac'] = mac


def parse_ip_link(output):
    """
    Given the full output from 'ip link [show]', parses it and returns a
    dictionary mapping each interface name to its settings.
    :param output: string or unicode
    :return: dict
    """
    interfaces = {}
    interface = None
    for line in output.splitlines():
        if re.match(r'^[0-9]', line):
            interface = _parse_interface_definition(line)
            if interface is not None:
                interfaces[interface['name']] = interface
        else:
            if interface is not None:
                _add_additional_interface_properties(interface, line)
    return interfaces

#------------------------------------------------------------------------------

_IP_ADDR_SPLIT_RE = re.compile("^[0-9]+: ", flags=re.MULTILINE)

def parse_ip_addr(data):
    """
    Parse addresses from 'ip addr' output
    """

    for iface in _IP_ADDR_SPLIT_RE.split(data.strip()):
        if not iface:
            continue
        lines = [l.strip() for l in iface.splitlines()]
        # XXX @ in name not supported
        name = lines.pop(0).partition(":")[0]
        info = {
            "ip-addresses": [],
            "hardware-address": None,
            }
        if '@' in name:
            name, parent = name.split('@')
            info['name'] = name
            info['parent'] = parent
        else:
            info['name'] = name
            info['parent'] = None

        for line in lines:
            words = line.split()
            if words[0].startswith("link/") and len(words) >= 2:
                info["hardware-address"] = words[1]
            elif words[0] in ("inet", "inet6"):
                addrtype = "ipv6" if words[0] == "inet6" else "ipv4"
                addr, _, prefix = words[1].partition("/")
                if prefix == '':
                    prefix = 128 if addrtype == "ipv6" else 32
                info["ip-addresses"].append({"ip-address-type": addrtype,
                        "ip-address": addr, "prefix": int(prefix)})
        yield info

#------------------------------------------------------------------------------

class BaseNetDevice(Interface, Application):
    __type__ = BaseResource

    # XXX note: ethtool only required if we need to get the pci address
    __package_names__ = ['ethtool']

    device_name = Attribute(String, description = 'Name of the NetDevice',
            default = lambda x : x._default_device_name(),
            max_size = MAX_DEVICE_NAME_SIZE)
    capacity = Attribute(Integer,
            description = 'Capacity for interface shaping (Mb/s)')
    mac_address = Attribute(String, description = 'Mac address of the device')
    ip4_address = Attribute(String, description = 'IP address of the device')
    ip6_address = Attribute(String, description = 'IPv6 address of the device')
    ip6_prefix = Attribute(Integer, description = 'Prefix for the IPv6 link', default=64)
    ip6_forwarding = Attribute(Bool, description = 'IPv6 forwarding', default = True)
    pci_address = Attribute(String,
            description = 'PCI bus address of the device',
            ro = True)
    promiscuous = Attribute(Bool, description = 'Promiscuous', default = False)
    up = Attribute(Bool, description = 'Promiscuous', default = True)
    netdevice_type = Attribute(String, description = 'Type of the netdevice',
            ro = True)
    prefix = Attribute(String, default = 'dev')
    rp_filter = Attribute(Bool, description = 'Reverse-path filtering enabled',
            default = True)

    #--------------------------------------------------------------------------
    # Constructor and Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        # Dummy member to store the other side of a VethPair
        # We use it to disable offloading on interfaces connected to VPP
        self.remote = None

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __get__(self):
        def check(rv):
            if not bool(rv):
                raise ResourceNotFound
        return BashTask(self.node, CMD_GET, {'netdevice' : self}, output=True,
                parse=check)

    __create__ = None

    def __delete__(self):
        return BashTask(self.node, CMD_DELETE, {'netdevice': self})

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def _get_mac_address(self):
        # Merge into parse_ip_link
        def parse(rv):
            assert rv is not None

            nds = parse_ip_link(rv.stdout)
            # This will raise an exception is the interface does not exist
            nd = nds[self.device_name]
            attrs = { 'mac_address': nd['mac'], }
            return attrs

        return BashTask(self.node, CMD_GET, {'netdevice' : self}, output=True,
                parse=parse)

    def _set_mac_address(self):
        return BashTask(self.node, CMD_SET_MAC_ADDRESS, {'netdevice': self})

    def _get_ip4_address(self):
        """
        NOTE: Incidently, this will also give the MAC address, as well as other
        attributes...
        """
        def parse(rv):
            attrs = dict()

            assert rv is not None

            nds = list(parse_ip_addr(rv.stdout))

            assert nds
            assert len(nds) <= 1

            nd = nds[0]

            assert nd['name'] == self.device_name

            attrs['mac_address'] = nd['hardware-address']

            # We assume a single IPv4 address for now...
            ips = [ip for ip in nd['ip-addresses']
                    if ip['ip-address-type'] == 'ipv4']
            if len(ips) >= 1:
                if len(ips) > 1:
                    log.warning('Keeping only first of many IP addresses...')
                ip = ips[0]
                attrs['ip4_address'] = ip['ip-address']
            else:
                attrs['ip4_address'] = None
            return attrs

        return BashTask(self.node, CMD_GET_IP_ADDRESS,
                {'netdevice': self}, parse=parse)

    def _set_ip4_address(self):
        if self.ip4_address is None:
            # Unset IP
            return BashTask(self.node, CMD_FLUSH_IP,
                    {'device_name': self.device_name, 'ip_version': IPV4})
        return BashTask(self.node, CMD_SET_IP4_ADDRESS, {'netdevice': self})

    def _get_ip6_address(self):
        """
        NOTE: Incidently, this will also give the MAC address, as well as other
        attributes...
        """
        def parse(rv):
            attrs = dict()

            assert rv is not None

            nds = list(parse_ip_addr(rv.stdout))

            assert nds
            assert len(nds) <= 1

            nd = nds[0]

            assert nd['name'] == self.device_name

            attrs['mac_address'] = nd['hardware-address']

            # We assume a single IPv4 address for now...
            ips = [ip for ip in nd['ip-addresses']
                    #We remove the link-local address that starts with fe80
                    if ip['ip-address-type'] == 'ipv6' and ip['ip-address'][:4] != 'fe80']
            if len(ips) >= 1:
                if len(ips) > 1:
                    log.warning('Keeping only first of many IPv6 addresses...')
                ip = ips[0]
                attrs['ip6_address'] = ip['ip-address']
                attrs['ip6_prefix']  = ip['prefix']
            else:
                attrs['ip6_address'] = None
            return attrs

        return BashTask(self.node, CMD_GET_IP_ADDRESS,
                {'netdevice': self}, parse=parse)

    _get_ip6_prefix = _get_ip6_address

    def _set_ip6_address(self):
        if self.ip6_address is None:
            # Unset IP
            return BashTask(self.node, CMD_FLUSH_IP,
                    {'device_name': self.device_name, 'ip_version': IPV6})
        return BashTask(self.node, CMD_SET_IP6_ADDRESS, {'netdevice': self})

    def _get_ip6_forwarding(self):
        def parse(rv):
            ret = {"ip6_forwarding" : False}
            if rv.stdout == "1":
                ret["ip6_forwarding"] = True
            return ret
        return BashTask(self.node, CMD_GET_IP6_FWD, {'netdevice': self}, parse=parse)

    def _set_ip6_forwarding(self):
        cmd = CMD_SET_IP6_FWD if self.ip6_forwarding else CMD_UNSET_IP6_FWD
        return BashTask(self.node, cmd, {'netdevice': self})

    @task
    def _get_promiscuous(self):
        return {'promiscuous': False}

    def _set_promiscuous(self):
        on_off = 'on' if self.promiscuous else 'off'
        return BashTask(self.node, CMD_SET_PROMISC,
                {'netdevice': self, 'on_off' : on_off})

    @task
    def _get_up(self):
        return {'up': False}

    def _set_up(self):
        up_down = 'up' if self.up else 'down'
        return BashTask(self.node, CMD_SET_UP,
                {'netdevice': self, 'up_down': up_down})

    @task
    def _get_capacity(self):
        return {'capacity': None}

    def _set_capacity(self):
        if self.capacity is None:
            log.warning('set_capacity(None) not implemented')
            return EmptyTask()

        # http://unix.stackexchange.com/questions/100785/bucket-size-in-tbf
        MBPS = 1000000
        KBPS = 1024
        BYTES = 8
        HZ = 250

        # Round to power of two... see manpage
        burst = math.ceil((((self.capacity * MBPS) / HZ) / BYTES) / KBPS)
        burst = 1 << (burst - 1).bit_length()

        return BashTask(self.node, CMD_SET_CAPACITY,
                {'netdevice': self, 'burst': burst})

    def _get_rp_filter(self):
        def parse(rv):
            lines = rv.stdout.splitlines()
            return (int(lines[0][-1]) + int(lines[1][-1]) > 0)
        return BashTask(self.node, CMD_GET_RP_FILTER, {'netdevice' :self},
                parse = parse)

    def _set_rp_filter(self):
        cmd = CMD_SET_RP_FILTER if self.rp_filter else CMD_UNSET_RP_FILTER
        return BashTask(self.node, cmd, {'netdevice' : self})

    #--------------------------------------------------------------------------
    # Internal methods
    #--------------------------------------------------------------------------

    def _remote_node_name(self):
        remote_interface = self._remote_interface()
        if remote_interface:
            return remote_interface.node.name
        else:
            rnd = ''.join(random.choice(string.ascii_uppercase + string.digits)
                    for _ in range(3))
            return 'unk{}'.format(rnd)

    def _remote_interface(self):
        if not self.channel:
            return None
        interfaces = self.channel.interfaces
        for interface in interfaces:
            if interface == self:
                continue
            return interface

    def _default_device_name(self):
        remote_node_name = self._remote_node_name()
        if remote_node_name:
            return remote_node_name
        else:
            return AddressManager().get('device_name', self,
                    prefix = self.prefix, scope = self.prefix)

#------------------------------------------------------------------------------

class NonTapBaseNetDevice(BaseNetDevice):
    # Tap devices for instance don't have offload
    offload = Attribute(Bool, description = 'Offload', default=True)

    #--------------------------------------------------------------------------
    # Attributes
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def _get_offload(self):
        return BashTask(self.node, CMD_GET_OFFLOAD, {'netdevice': self},
                parse = lambda rv : rv.stdout.strip() == 'on')

    def _set_offload(self):
        cmd = None
        if self.offload:
            cmd = CMD_SET_OFFLOAD
        else:
            cmd = CMD_UNSET_OFFLOAD
        return BashTask(self.node, cmd, {'netdevice' : self})

#------------------------------------------------------------------------------

class NetDevice(NonTapBaseNetDevice):

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __create__(self):
        return BashTask(self.node, CMD_CREATE, {'netdevice': self})

#------------------------------------------------------------------------------

class SlaveBaseNetDevice(BaseNetDevice):
    parent = Attribute(NetDevice, description = 'Parent NetDevice')

    host = Attribute(NetDevice, description = 'Host interface',
            default = lambda x : x._default_host())

    def _default_host(self):
        if self.node.__class__.__name__ == 'LxcContainer':
            host = self.node.node
        else:
            host = self.node
        max_len = MAX_DEVICE_NAME_SIZE - len(self.node.name) - 1
        device_name = self.device_name[:max_len]

        return NetDevice(node = host,
                device_name = '{}-{}'.format(self.node.name, device_name),
                managed = False)

#------------------------------------------------------------------------------

class SlaveNetDevice(SlaveBaseNetDevice):

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __create__(self):
        return BashTask(self.node, CMD_CREATE_PARENT, {'netdevice': self})
