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
        scenario = None
        node_list, net, ndn, mob, cluster = None, None, None, None, None

        parser = ArgumentParser(description=textcolor('green', "Batch usage of VICN."))
        parser.add_argument('-s', metavar='configuration_file_path',
                            help="JSON file containing the topology")
        parser.add_argument('-n', metavar='n_times', type=int,  help='Execute the test multiple times')
        parser.add_argument('-x', action='store_false', help='No automatic execution')

        arguments = parser.parse_args()
        args = vars(arguments)


        for option in args.keys():
            if args[option] is not None:
                if option == "s":
                    print(" * Loading the configuration file at {0}".format(args[option]))
                    scenario = args[option]
                elif option == "t" and args[option] is True:
                    background = True
                elif option == "x" and args[option] is True:
                    setup = True
                elif option == "n":
                    n_times = args[option]

        self._api = API()
        self._api.configure(scenario, setup)

        if node_list is not None:
            ResourceManager().set(node_list)

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
