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

import os, sys, time, asyncio, argparse, shutil, threading, asyncio, logging
import traceback

log = logging.getLogger(__name__)

PATH=os.path.join(os.path.dirname(__file__), os.path.pardir, os.path.pardir)
sys.path.insert(0, os.path.abspath(PATH))

from netmodel.model.query       import Query
from netmodel.util.daemon       import Daemon
from netmodel.util.log          import textcolor, initialize_logging
from vicn.clients.command_line  import VICNCmd
from vicn.core.api              import API
from vicn.core.resource_mgr     import ResourceManager
from vicn.resource.node         import Node

class ArgumentParser(argparse.ArgumentParser):
    def error(self, message):
        self._print_message(textcolor('red', '{prog}: error: {message}\n'.format(prog=self.prog, message=message)),
                            sys.stderr)

        self.print_usage(sys.stdout)
        sys.exit(-1)

class VICNDaemon(Daemon):
    def initialize(self):
        # FIXME UGLY
        n_times = 1
        background = False
        setup = False

        parser = ArgumentParser(description=textcolor('green', "Batch usage of VICN."))
        parser.add_argument('-s', '--scenario', metavar='configuration_file_path',
                action='append',
                help="JSON file containing the topology")
        parser.add_argument('-z', '--identifier', metavar='identifier', type=str, help='Experiment identifier')
        parser.add_argument('-x', '--no-execute', action='store_false', help='Configure only, no automatic execution')
        parser.add_argument('-c', '--clean', action='store_true', help='Clean deployment before setup')
        parser.add_argument('-C', '--clean-only', action='store_true', help='Clean only')

        arguments = parser.parse_args()

        scenario = arguments.scenario
        if not scenario:
            log.error('No scenario specified')
            sys.exit(-1)

        identifier = arguments.identifier or "default"
        clean = arguments.clean or arguments.clean_only
        execute = not arguments.clean_only or arguments.no_execute

        self._api = API()
        self._api.configure(scenario)

        if clean:
            self._api.teardown()
        if execute:
            self._api.setup(commit = True)

    def main(self):
        """
        Main asyncio loop.

        Ctrl+C properly terminates the loop by terminating all running
        instances.
        """
        loop = asyncio.get_event_loop()
        try:
            loop.run_forever()
        except KeyboardInterrupt:
            pass
        except Exception as e:
            import traceback
            traceback.print_exc()
        finally:
            loop.stop()
            self._api.terminate()

def main():
    initialize_logging()
    VICNDaemon('vicn', no_daemon = True).start()

if __name__ == "__main__":
    main()
