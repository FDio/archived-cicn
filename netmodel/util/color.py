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

# ANSI escape codes for terminals.
#  X11 xterm: always works, all platforms
#  cygwin dosbox: run through |cat and then colors work
#  linux: works on console & gnome-terminal
#  mac: untested
 
BLACK      = "\033[0;30m"
BLUE       = "\033[0;34m"
GREEN      = "\033[0;32m"
CYAN       = "\033[0;36m"
RED        = "\033[0;31m"
PURPLE     = "\033[0;35m"
BROWN      = "\033[0;33m"
GRAY       = "\033[0;37m"
BOLDGRAY   = "\033[1;30m"
BOLDBLUE   = "\033[1;34m"
BOLDGREEN  = "\033[1;32m"
BOLDCYAN   = "\033[1;36m"
BOLDRED    = "\033[1;31m"
BOLDPURPLE = "\033[1;35m"
BOLDYELLOW = "\033[1;33m"
WHITE      = "\033[1;37m"

MYCYAN     = "\033[96m"
MYGREEN    = '\033[92m'
MYBLUE     = '\033[94m'
MYWARNING  = '\033[93m'
MYRED      = '\033[91m'
MYHEADER   = '\033[95m'
MYEND      = '\033[0m'
 
NORMAL = "\033[0m"

colors = {
    'white':        "\033[1;37m",
    'yellow':       "\033[1;33m",
    'green':        "\033[1;32m",
    'blue':         "\033[1;34m",
    'cyan':         "\033[1;36m",
    'red':          "\033[1;31m",
    'magenta':      "\033[1;35m",
    'black':        "\033[1;30m",
    'darkwhite':    "\033[0;37m",
    'darkyellow':   "\033[0;33m",
    'darkgreen':    "\033[0;32m",
    'darkblue':     "\033[0;34m",
    'darkcyan':     "\033[0;36m",
    'darkred':      "\033[0;31m",
    'darkmagenta':  "\033[0;35m",
    'darkblack':    "\033[0;30m",
    'off':          "\033[0;0m"
}

def textcolor(color, string):
    """
    This function is useful to output information to the stdout by exploiting
    different colors, depending on the result of the last command executed.

    It is possible to chose one of the following colors:
        - white
        - yellow
        - green
        - blue
        - cyan
        - red
        - magenta
        - black
        - darkwhite
        - darkyellow
        - darkgreen
        - darkblue
        - darkcyan
        - darkred
        - darkmagenta
        - darkblack
        - off

    :param color: The color of the output string, chosen from the previous
        list.
    :param string: The string to color
    :return: The colored string if the color is valid, the original string
        otherwise.
    """

    try:
        return colors[color] + string + colors['off']
    except:
        return string

if __name__ == '__main__':
    # Display color names in their color
    for name, color in locals().items():
        if name.startswith('__'): continue
        print(color, name, MYEND)

