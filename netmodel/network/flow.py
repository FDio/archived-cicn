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

class Flow:
    def __init__(self, source, destination):
        """
        Constructor.
        Args:
            source: The source Prefix of this Flow.
            destination: The destination Prefix of this Flow.
        """
        self.source = source
        self.destination = destination

    def get_reverse(self):
        """
        Make the reverse Flow of this Flow.
        Returns:
            The reverse Flow.
        """
        return Flow(self.destination, self.source)

    def __eq__(self, other):
        """
        Tests whether two Flows are equal or not.
        Args:
            other: A Flow instance.
        Returns:
            True iif self == other.
        """
        if self.source and other.source and self.source != other.source:
            return False
        if self.destination and other.destination and self.destination != other.destination:
            return False
        return True

    def __hash__(self):
        # Order is important
        return hash((self.source, self.destination))

    def __repr__(self):
        return "<Flow %s -> %s>" % (self.source, self.destination)
