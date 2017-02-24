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

import argparse
import cmd
import logging
import os
import shlex
import sys
import traceback
import time

from vicn.core.api              import API
from netmodel.util.log         import textcolor

# FIXME 
log_file = "/tmp/vicn.log"

log = logging.getLogger(__name__)

# FIXME
EXPERIMENT_ID = 'vicn'

welcome = textcolor("cyan",
                       """
                       ================================================
                                       Welcome to VICN
                       ================================================
                                 * Type 'help' for a brief help
                       """)

class ArgumentParser(argparse.ArgumentParser):
    """
    The class ArgumentParser extends the system class :class:`argparse.ArgumentParser`
    for what concerns the error management.
    """

    def error(self, message):
        """
        Print the error and the list of available commands.

        :param message: The error message
        :return:
        """
        self.print_usage(sys.stderr)
        self._print_message('{prog}: error: {message}\n'.format(prog=self.prog, message=message), sys.stderr)


def raw_input(prompt):
    """
    Print a message on command line and get user input
    :param prompt: message to be shown
    :return: the input line (without the '\n' character)
    """
    print(prompt, end='', flush=True)
    return sys.stdin.readline()[:-1]


class MyCmd(cmd.Cmd, object):
    """
    The class MyCmd inherits from the system class :class:`cmd.Cmd` and adds the following extensions:
        - The preloop prints the welcome message::

                       ================================================
                                       Welcome to VICN
                       ================================================
                                * Type 'help' for a brief help
        - The list of available commands does not show the EOF command (ctrl + D)
        - The empty line command does not execute the last command
        - It override the class variables prompt, doc_header and ruler

    :cvar prompt: the prompt of the CLI. Value: "VICN >"
    :cvar doc_header: the header for the list of available commands. Value: "Available commands (type help <command> for more info)"
    :cvar ruler: The character used to draw separator lines under the help-message headers. Value: "-"

    """
    prompt = textcolor('blue', 'VICN > ')
    doc_header = textcolor("green", 'Available commands (type help <command> for more info)')
    ruler = '-'

    def preloop(self):
        """
        Print the welcome message before starting the CLI loop.

        :return:
        """
        print(welcome)

    def get_names(self):
        """
        Return the list of available commands.

        :return: The list of commands without the EOF (Ctrl + D) function.
        """
        names = super(MyCmd, self).get_names()
        if "do_EOF" in names:
            names.remove("do_EOF")
        return names

    def emptyline(self):
        """
        Override the base function in order to avoid re-executing the last
        command.
        """
        pass


class VICNCmd(MyCmd):

    def __init__(self, api):
        """
        Instantiate a CLI for vicn. If all the inputs are valid (not None) it
        configures the experiment with the received parameters, otherwise the
        CLI will be started without configuration. The user later has to
        specify the path of the experiment folder through the configure
        command.

        :return:
        """
        super(MyCmd, self).__init__()

        log.debug("Creating CLI instance.")

        self._api = api

    def run(self):
        """Runs the CLI main loop in a thread
        """
        while True:
            try:
                self.cmdloop()
                break
            except KeyboardInterrupt:
                self.exit_gracefully()
            except Exception as e:
                traceback.print_exc()
                self.exit()

    def terminate(self):
        """
        """

    def do_EOF(self, line):
        """
        Type Ctrl+D to exit from User CLI
        """
        log.debug("Exiting from vicn.")
        self.exit_gracefully()
        return True

    def do_clear(self, line):
        """
        Clear the screen.
        """
        os.system('clear')

    def do_quit(self, line):
        """
        Quit from VICN and clean the testbed.
        """

        log.info("Exit from VICN")
        self.exit()

    def do_configure(self, line):
        """
        Configure the experiment by reading the configuration file specified.
        Available command line arguments:
            -c <conf_directory_path>: Path to the directory containing the
                configuration files
            -s Show the current VICN configuration
        """

        log.debug("Parsing configuration files")
        log.debug("Parsing configuration files")

        try:
            args = self.configure_parser.parse_args(shlex.split(line))
        except TypeError:
            return
        except SystemExit:
            return

        if args is None:
            log.warning("You must specify the directory with the input files,"\
                    "or write -s to show the configuration file!")
            return

        if args.show:

            log.debug("Showing the list of nodes")

            if self.node_list is not None:
                for node in self.node_list.values():
                    print(node)
            else:
                log.warning("No configuration!")
        elif args.conf_dir is not None:
            self._api.configure(args.conf_dir)

        else:
            log.warning("No options found.")

    def help_configure(self):
        """
        Print the help for the configure command.
        """
        self.configure_parser.print_help()

    def complete_configure(self, text, line, begidx, endidx):
        """
        Auto completion for the file name. This function is really useful when
        the user has to specify the path to the experiment folder in the
        configure command.

        :param text: String prefix we are attempting to match
        :param line: Current input line with leading whitespace removed
        :param begidx: beginning index of the prefix text, which could be used
                to provide different completion depending upon which position
                the argument is in.
        :param endidx: ending index of the prefix text, which could be used to
                provide different completion depending upon which position the
                argument is in.
        :return: List of matches for the current file path
        """

        line = line.split()
        if len(line) == 2 and begidx == endidx:
            filename = ''
            path = './'
        elif len(line) == 3:
            path = line[2]
            if '/' in path:
                i = path.rfind('/')
                filename = path[i + 1:]
                path = path[0:i + 1]
            else:
                filename = path
                path = './'
        else:
            return

        ls = os.listdir(path)
        ls.sort()

        ls = ls[:]
        for i in range(len(ls)):
            if os.path.isdir(os.path.join(path, ls[i])):
                ls[i] += '/'
        if filename == '':
            return ls
        else:
            return [f for f in ls if f.startswith(filename)]

    def do_print(self, line):
        ll = {}
        for node in self.node_list.keys():
            print(node)

    def evaluate(self, line):
        dic = SQLParser().parse(command)
        if not dic:
            raise RuntimeError("Can't parse input command: %s" % command)

        query = Query.from_dict(dic)

        return self._api.execute(query, annotation)


    def do_select(self, line):
        """
        Query interface: select
        """
        query = self._api.parse_query('select {}'.format(line))
        results = self._api.execute(query)
        for result in results:
            print(result)

    def do_setup(self, line):
        assert not line
        self._api.setup()

    def do_teardown(self, line):
        assert not line
        self._api.teardown()

    def exit(self):
        """
        Exit from the program by cleaning the environment.
        """

        print(textcolor("green", "Cleaning the cluster. Wait.."))
        sys.exit()

    def exit_gracefully(self):
        """
        Ask the user if he really wants to exit from the cmd loop
        :return:
        """

        try:
            inp = raw_input(textcolor("yellow", "\nReally quit? (y/n) "))
            if inp.lower().startswith('y'):
                self.exit()
        except KeyboardInterrupt:
            print(textcolor("red", "\nOk ok, quitting"))
            self.exit()
