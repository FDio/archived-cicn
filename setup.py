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

import os
from glob           import glob
from platform       import dist

# XXX
from setuptools import find_packages, setup

# Versions should comply with PEP440. For a discussion on single-sourcing
# the version across setup.py and the project code, see
# https://packaging.python.org/en/latest/single_source_version.html
with open(os.path.join(os.path.dirname(__file__), 'VERSION')) as version_file:
    version = version_file.read().strip()

# Like VERSION, this file is made available through MANIFEST.in
with open('README.md') as f:
    long_description = f.read()

# XXX TODO
required_modules = list()

data_files = list()

data_files.extend([
    ("/lib/systemd/system/", ["etc/netmon.service"]),
    ("/etc/init/", ["etc/netmon.conf"]),
])


setup(
    name                = 'vicn',
    version             = version,
    description         = 'vICN experiment controller',
    long_description    = long_description,
    license             = 'Apache 2.0',

    download_url        = 'https://gerrit.fd.io/r/cicn',
    url                 = 'https://wiki.fd.io/view/Vicn',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
	'Development Status :: 3 - Alpha',
	'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
	'Topic :: Software Development :: Build Tools',
        'Operating System :: POSIX :: Linux',
        'Operating System :: MacOS :: MacOS X',
        'License :: OSI Approved :: Apache Software License',
	'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    keywords                = 'Experiment Controller; Orchestrator; ICN; LXC; Containers',
    platforms               = "Linux, OSX",
    packages                = find_packages(),
    data_files              = data_files,

    install_requires        = required_modules,

    # To provide executable scripts, use entry points in preference to the
    # "scripts" keyword. Entry points provide cross-platform support and allow
    # pip to create the appropriate form of executable for the target platform.
    entry_points = {
	'console_scripts': [
            'vicn  = vicn.bin.vicn:main',
	],
    },
)

setup(
    name = 'netmon',
    version             = version,
    description         = 'Netmon',
    long_description    = long_description,
    license             = 'Apache 2.0',

    download_url        = 'https://gerrit.fd.io/r/cicn',
    url                 = 'https://wiki.fd.io/view/Vicn',

    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
	'Development Status :: 3 - Alpha',
	'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
	'Topic :: Software Development :: Build Tools',
        'Operating System :: POSIX :: Linux',
        'Operating System :: MacOS :: MacOS X',
        'License :: OSI Approved :: Apache Software License',
	'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    keywords                = 'Experiment Controller; Orchestrator; ICN; LXC; Containers',
    platforms               = "Linux, OSX",
    packages                = find_packages(),
    data_files              = data_files,

    install_requires        = required_modules,

    # To provide executable scripts, use entry points in preference to the
    # "scripts" keyword. Entry points provide cross-platform support and allow
    # pip to create the appropriate form of executable for the target platform.
    entry_points = {
	'console_scripts': [
            'netmon  = netmon.bin.netmon:main',
	],
    },
)

