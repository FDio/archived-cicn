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
import logging.config
import os
import sys

from netmodel.util.file import ensure_writable_directory

# Monkey-patch logging.FileHandler to support expanduser()
oldFileHandler = logging.FileHandler
class vICNFileHandler(oldFileHandler):
    def __init__(self, filename, mode='a', encoding=None, delay=False):
        filename = os.path.expanduser(filename)
        ensure_writable_directory(os.path.dirname(filename))
        super().__init__(filename, mode, encoding, delay)
logging.FileHandler = vICNFileHandler

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

    It is possible to chose one of the following colors: white, yellow, green,
    blue, cyan, red, magenta, black, darkwhite, darkyellow, darkgreen,
    darkblue, darkcyan, darkred, darkmagenta, darkblack, off

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

FMT = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'

MAP_LEVELNAME_COLOR = {
    'DEBUG'     : 'off',
    'INFO'      : 'cyan',
    'WARNING'   : 'yellow',
    'ERROR'     : 'red',
    'CRITICAL'  : 'red'
}

DEFAULT_COLOR = 'blue'

DEPRECATED_REPEAT = False
DEPRECATED_DONE = list()

DEBUG_PATHS = 'vicn'

class DebugPathsFilter:
    def __init__(self, debug_paths):
        self._debug_paths = debug_paths

    def filter(self, record):
        return record.levelname != 'DEBUG' or record.name in self._debug_paths

class ColorFormatter(logging.Formatter):
    def __init__(self, *args, debug_paths = None, **kwargs):
        self._debug_paths = debug_paths
        super().__init__(*args, **kwargs)

    def format(self, record):
        formatted = super().format(record)

        try:
            color = record.category
        except AttributeError:
            color = MAP_LEVELNAME_COLOR.get(record.levelname, DEFAULT_COLOR)

        return textcolor(color, formatted)

def initialize_logging():
    # Use logger config
    config_path = os.path.join(os.path.dirname(__file__),
            os.path.pardir, os.path.pardir, 'config', 'logging.conf')
    if os.path.exists(config_path):
        logging.config.fileConfig(config_path, disable_existing_loggers=False)


    root = logging.getLogger()
    root.setLevel(logging.DEBUG)

    # Stdout handler
    ch = logging.StreamHandler(sys.stdout)
    ch.setLevel(logging.INFO)
    #formatter = logging.Formatter(FMT)
    formatter = ColorFormatter(FMT, debug_paths = DEBUG_PATHS)
    ch.setFormatter(formatter)
    ch.addFilter(DebugPathsFilter(DEBUG_PATHS))
    root.addHandler(ch)
