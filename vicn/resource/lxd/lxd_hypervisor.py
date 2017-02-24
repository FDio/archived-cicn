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

#-------------------------------------------------------------------------------
# NOTES
#-------------------------------------------------------------------------------
#  - lxd >= 2.0.4 is required
#      daemon/container: Remember the return code in the non wait-for-websocket
#      case (Issue #2243)
#  - Reference: https://github.com/lxc/lxd/tree/master/doc
#-------------------------------------------------------------------------------

import logging
import os
from pylxd                            import Client
from pylxd.exceptions                 import LXDAPIException

from netmodel.model.type              import String, Integer
from vicn.core.attribute              import Attribute, Multiplicity, Reference
from vicn.core.exception              import ResourceNotFound
from vicn.core.resource               import Resource
from vicn.core.task                   import BashTask, task
from vicn.resource.linux.application  import LinuxApplication as Application
from vicn.resource.linux.service      import Service
from vicn.resource.linux.certificate  import Certificate

# Suppress non-important logging messages from requests and urllib3
logging.getLogger("requests").setLevel(logging.WARNING)
logging.getLogger("urllib3").setLevel(logging.WARNING)
log = logging.getLogger(__name__)

# FIXME use system-wide files
DEFAULT_CERT_PATH = os.path.join(os.path.dirname(__file__),
        '..', '..', '..', 'config', 'lxd_client_cert', 'client_cert.pem')
DEFAULT_KEY_PATH = os.path.join(os.path.dirname(__file__),
        '..', '..', '..', 'config', 'lxd_client_cert', 'client_key.pem')

# FIXME hardcoded password for LXD server
DEFAULT_TRUST_PASSWORD = 'vicn'

DEFAULT_LXD_STORAGE = 100 # GB

# Commands used to interact with the LXD hypervisor
CMD_LXD_CHECK_INIT = 'lsof -i:{lxd.lxd_port}'

CMD_LXD_INIT_BASE = 'lxd init --auto '
CMD_LXD_INIT='''
{base}
lxc profile unset default environment.http_proxy
lxc profile unset default user.network_mode
'''

#------------------------------------------------------------------------------
# Subresources
#------------------------------------------------------------------------------

class LxdInit(Application):
    __package_names__ = ['lxd', 'zfsutils-linux', 'lsof']

    def __get__(self):
        return BashTask(self.owner.node, CMD_LXD_CHECK_INIT, 
                {'lxd': self.owner})

    def __create__(self):
        cmd_params = {
            'storage-backend'       : self.owner.storage_backend,
            'network-port'          : self.owner.lxd_port,
            'network-address'       : '0.0.0.0',
            'trust-password'        : DEFAULT_TRUST_PASSWORD,
        }

        if self.owner.storage_backend == 'zfs':
            cmd_params['storage-pool'] = self.owner.zfs_pool

            # zpool list -H -o name,cap
            # don't create it if it exists
            zfs_pool_exists = True

            if zfs_pool_exists:
                cmd_params['storage-create-loop'] = self.owner.storage_size
        elif self.owner.storage_backend == 'dir':
            raise NotImplementedError 
        else:
            raise NotImplementedError 
        cmd = CMD_LXD_INIT_BASE + ' '.join('--{}={}'.format(k, v) 
                for k, v in cmd_params.items())

        # error: Failed to create the ZFS pool: The ZFS modules are not loaded.
        # Try running '/sbin/modprobe zfs' as root to load them.
        # zfs-dkms in the host
        return BashTask(self.owner.node, CMD_LXD_INIT, {'base': cmd},
                as_root = True)

    def __delete__(self):
        raise NotImplementedError

class LxdInstallCert(Resource):
    certificate = Attribute(Certificate, mandatory = True)

    @task
    def __get__(self):
        try:
            self.owner.client.certificates.all()
        except LXDAPIException as e:
            if e.response.raw.status == 403:
                raise ResourceNotFound
            raise
        except Exception:
            # Missing certificates raises an exception
            raise ResourceNotFound
            

    @task
    def __create__(self):
        """
        Some operations with containers requires the client to be trusted by 
        the server. So at the beginning we have to upload a (self signed)
        client certificate for the LXD daemon.
        """
        log.info('Adding certificate on LXD')
        self.owner.client.authenticate(DEFAULT_TRUST_PASSWORD)
        if not self.owner.client.trusted:
            raise Exception

#------------------------------------------------------------------------------

class LxdHypervisor(Service):
    """
    Resource: LxdHypervisor

    Manages a LXD hypervisor, accessible through a REST API.
    """
    __service_name__ = 'lxd'

    lxd_port = Attribute(Integer, description = 'LXD REST API port', 
            default = 8443)
    storage_backend = Attribute(String, description = 'Storage backend',
            default = 'zfs',
            choices = ['zfs'])
    storage_size = Attribute(Integer, description = 'Storage size',
            default = DEFAULT_LXD_STORAGE) # GB
    zfs_pool = Attribute(String, description = 'ZFS pool', 
            default='vicn')

    # Just overload attribute with a new reverse
    node = Attribute(
            reverse_name = 'lxd_hypervisor',
            reverse_description = 'LXD hypervisor',
            reverse_auto = True,
            multiplicity = Multiplicity.OneToOne)

    #--------------------------------------------------------------------------
    # Constructor / Accessors
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._client = None
        self._images = None

    @property
    def client(self):
        if not self._client:
            self._client = Client(endpoint = self._get_server_url(),
                    cert=(DEFAULT_CERT_PATH, DEFAULT_KEY_PATH),
                    verify=False)
        return self._client

    #--------------------------------------------------------------------------
    # Resource lifecycle
    #--------------------------------------------------------------------------

    def __subresources__(self):
        lxd_init = LxdInit(owner=self, node = self.node)
        lxd_local_cert = Certificate(node = Reference(self, 'node'),
                cert = DEFAULT_CERT_PATH,
                key = DEFAULT_KEY_PATH, 
                owner = self)
        lxd_cert_install = LxdInstallCert(node = Reference(self, 'node'),
                certificate = lxd_local_cert,
                owner = self)

        return (lxd_init | lxd_local_cert) > lxd_cert_install

    #--------------------------------------------------------------------------
    # Private methods
    #--------------------------------------------------------------------------

    def _get_server_url(self):
        return 'https://{0}:{1}'.format(self.node.hostname, self.lxd_port)

    #--------------------------------------------------------------------------
    # Public interface
    #--------------------------------------------------------------------------

    @property
    def images(self):
        """
        This method caches available images to minimize the number of queries
        done when creating multiple containers.
        """
        if not self._images:
            self._images = self.node.lxd_hypervisor.client.images.all() 
        return self._images

    @property
    def aliases(self):
        return [alias for image in self.images for alias in image.aliases]
