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

import random
import string

# Separator for components of the UUID
UUID_SEP = '-'

# Length of the random component of the UUID
UUID_LEN = 5

class UUID:
    def __init__(self, name, cls):
        self._uuid = self._make_uuid(name, cls)

    def _make_uuid(self, name, cls):
        """Generate a unique resource identifier

        The UUID consists in the type of the resource, to which is added a
        random identifier of length UUID_LEN. Components of the UUID are
        separated by UUID_SEP.
        """
        uuid = ''.join(random.choice(string.ascii_uppercase + string.digits)
                for _ in range(UUID_LEN))
        if name:
            uuid = name # + UUID_SEP + uuid
        return UUID_SEP.join([cls.__name__, uuid])

    def __repr__(self):
        return '<UUID {}>'.format(self._uuid)

    def __lt__(self, other):
        return self._uuid < other._uuid

    __str__ = __repr__
