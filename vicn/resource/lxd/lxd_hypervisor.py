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
import os
from pylxd                            import Client
from pylxd.exceptions                 import LXDAPIException

from netmodel.model.type              import String, Integer
from vicn.core.attribute              import Attribute, Multiplicity, Reference
from vicn.core.exception              import ResourceNotFound
from vicn.core.resource               import Resource
from vicn.core.task                   import EmptyTask, BashTask, task
from vicn.core.task                   import inherit_parent, override_parent
from vicn.resource.linux.application  import LinuxApplication as Application
from vicn.resource.linux.service      import Service
from vicn.resource.linux.certificate  import Certificate
from vicn.resource.lxd.lxd_profile    import LxdProfile

# Suppress non-important logging messages from requests and urllib3
logging.getLogger("requests").setLevel(logging.WARNING)
logging.captureWarnings(True)
#This one is for urllib, it disables the InsecureRequestWarning
logging.getLogger("py.warnings").setLevel(logging.ERROR)
log = logging.getLogger(__name__)

DEFAULT_CERT_PATH = os.path.expanduser(os.path.join(
        '~', '.vicn', 'lxd_client_cert', 'client_cert.pem'))
DEFAULT_KEY_PATH = os.path.expanduser(os.path.join(
        '~', '.vicn', 'lxd_client_cert', 'client_key.pem'))

LXD_TRUST_PWD_DEFAULT = 'vicn'

LXD_STORAGE_SIZE_DEFAULT = 100 # GB
LXD_NETWORK_DEFAULT = 'lxdbr-vicn'
LXD_PROFILE_NAME_DEFAULT = 'vicn'

ZFS_DEFAULT_POOL_NAME = 'vicn'
# Commands used to interact with the LXD hypervisor
CMD_LXD_CHECK_INIT = 'lsof -i:{lxd.lxd_port}'

CMD_LXD_INIT_BASE = 'lxd init --auto '

CMD_LXD_NETWORK_GET = 'lxc network list | grep {lxd_hypervisor.network}'
CMD_LXD_NETWORK_SET = 'lxc network create {lxd_hypervisor.network} || true'

#------------------------------------------------------------------------------
# Subresources
#------------------------------------------------------------------------------

class LxdInit(Application):
    __package_names__ = ['lxd', 'zfsutils-linux', 'lsof']

    @inherit_parent
    def __get__(self):
        return BashTask(self.owner.node, CMD_LXD_CHECK_INIT,
                {'lxd': self.owner})

    @inherit_parent
    def __create__(self):
        cmd_params = {
            'storage-backend'       : self.owner.storage_backend,
            'network-port'          : self.owner.lxd_port,
            'network-address'       : '0.0.0.0',
            'trust-password'        : self.owner.trust_password,
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
        return BashTask(self.owner.node, cmd, as_root = True)

    @inherit_parent
    def __delete__(self):
        raise NotImplementedError

class LxdInstallCert(Resource):
    certificate = Attribute(Certificate, mandatory = True)

    @inherit_parent
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


    @inherit_parent
    @task
    def __create__(self):
        """
        Some operations with containers requires the client to be trusted by
        the server. So at the beginning we have to upload a (self signed)
        client certificate for the LXD daemon.
        """
        log.info('Adding certificate on LXD')
        self.owner.client.authenticate(self.owner.trust_password)
        if not self.owner.client.trusted:
            raise Exception

#------------------------------------------------------------------------------

LxdStorageType = String.restrict(choices=('zfs'))

class LxdHypervisor(Service):
    """
    Resource: LxdHypervisor

    Manages a LXD hypervisor, accessible through a REST API.
    """
    __service_name__ = 'lxd'

    lxd_port = Attribute(Integer, description = 'LXD REST API port',
            default = 8443)
    storage_backend = Attribute(LxdStorageType, description = 'Storage backend',
            default = 'zfs')
    storage_size = Attribute(Integer, description = 'Storage size',
            default = LXD_STORAGE_SIZE_DEFAULT) # GB
    zfs_pool = Attribute(String, description = 'ZFS pool',
            default=ZFS_DEFAULT_POOL_NAME)
    network  = Attribute(String, description = 'LXD network name',
            default=LXD_NETWORK_DEFAULT)
    trust_password = Attribute(String, description = 'Trust password for the LXD server',
            default=LXD_TRUST_PWD_DEFAULT)

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

    @inherit_parent
    def __subresources__(self):
        lxd_init = LxdInit(owner=self, node = self.node)
        lxd_local_cert = Certificate(node = Reference(self, 'node'),
                cert = DEFAULT_CERT_PATH,
                key = DEFAULT_KEY_PATH,
                owner = self)
        lxd_cert_install = LxdInstallCert(certificate = lxd_local_cert,
                owner = self)
        # XXX BUG network has to exist before profile, although as an attribute it
        # will be setup after
        lxd_vicn_profile = LxdProfile(name=LXD_PROFILE_NAME_DEFAULT,
                                      node=self.node,
                                      description='vICN profile',
                                      network=self.network,
                                      pool=self.zfs_pool)

        return (lxd_init | lxd_local_cert) > (lxd_vicn_profile | lxd_cert_install)

    @override_parent
    def __create__(self):
        log.warning('Not restarting LXD')
        return EmptyTask()

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

    @task
    def _get_network(self):
        return None #XXX We assume it's always nothing

    def _set_network(self):
        return BashTask(self.node, CMD_LXD_NETWORK_SET, {'lxd_hypervisor': self})
