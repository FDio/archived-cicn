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

import errno
import logging
import os
import tempfile

log = logging.getLogger(__name__)

def mkdir(directory):
    """
    Create a directory (mkdir -p).
    Args:
        directory: A String containing an absolute path.
    Raises:
        OSError: If the directory cannot be created.
    """
    try:
        if not os.path.exists(directory):
            log.info("Creating '%s' directory" % directory)
        os.makedirs(directory)
    except OSError as e:
        if e.errno == errno.EEXIST and os.path.isdir(directory):
            pass
        else:
            raise OSError("Cannot mkdir %s: %s" % (directory, e))

def check_writable_directory(directory):
    """
    Tests whether a directory is writable.
    Args:
        directory: A String containing an absolute path.
    Raises:
        RuntimeError: If the directory does not exists or isn't writable.
    """
    if not os.path.exists(directory):
        raise RuntimeError("Directory '%s' does not exists" % directory)
    if not os.access(directory, os.W_OK | os.X_OK):
        raise RuntimeError("Directory '%s' is not writable" % directory)
    try:
        with tempfile.TemporaryFile(dir = directory):
            pass
    except Exception as e:
        raise RuntimeError("Cannot write into directory '%s': %s" % (directory, e))

def ensure_writable_directory(directory):
    """
    Tests whether a directory exists and is writable. If not,
    try to create such a directory.
    Args:
        directory: A String containing an absolute path.
    Raises:
        RuntimeError: If the directory does not exists and cannot be created.
    """
    try:
        check_writable_directory(directory)
    except RuntimeError as e:
        mkdir(directory)
