#! /usr/bin/env python
# Copyright (c) 2017 Cisco and/or its affiliates.
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

#
import sys
import os
import subprocess
'''
This programme takes a previously generated LongBow production (see longbow-preprocess.py) as input
and generates corresponding C code as a template for a complete test runner for that production.
'''

ansiRed = "\x1b[31m";
ansiGreen = "\x1b[32m";
ansiYellow = "\x1b[33;1m";
ansiOrange = "\x1b[33m";
ansiReset = "\x1b[0m";


def ANSITerminal_printchars(color, chars):
    if color == "red":
        return ansiRed + chars + ansiReset
    if color == "green":
        return ansiGreen + chars + ansiReset
    if color == "yellow":
        return ansiYellow + chars + ansiReset
    if color == "orange":
        return ansiOrange + chars + ansiReset
    return chars


class LongBowAnsi:
    def __init__(self, input=sys.stdin):
        self.input = input
        return

    def tokenise(self, line):
        fields = line.split(":", 2)
        fields[0] = fields[0].strip()
        return fields

    def colourise(self):
        lines = self.input.read().splitlines()
        for line in lines:
            fields = self.tokenise(line)
            if len(fields) == 3:
                if fields[0] == "#####":
                    print ANSITerminal_printchars("red", fields[1]), ANSITerminal_printchars("red", fields[2])
                    pass
                elif fields[0] == "$$$$$":
                    print ANSITerminal_printchars("yellow", fields[1]), ANSITerminal_printchars("yellow", fields[2])
                    pass
                else:
                    print ANSITerminal_printchars("green", fields[1]), ANSITerminal_printchars("green", fields[2])
                    pass
                pass
            pass
        return


if __name__ == '__main__':
    outputFileName = None
   
    if len(sys.argv) != 2:
        print "Usage: longbow-ansigov.py file.gcov"
        sys.exit(1)

    with open(sys.argv[1], 'r') as f:
        longBowAnsi = LongBowAnsi(f)
        longBowAnsi.colourise()
    f.close()

    pass
