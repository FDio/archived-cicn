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

# see also: http://www.jejik.com/files/examples/daemon3x.py

# This is used to import the daemon package instead of the local module which is
# named identically...
from __future__             import absolute_import

import os
import sys
import time
import logging
import atexit
import signal
import lockfile
import traceback

log = logging.getLogger(__name__)

class Daemon:

    #--------------------------------------------------------------------------
    # Checks
    #--------------------------------------------------------------------------

    def check_python_daemon(self):
        """
        Check whether python-daemon is properly installed.
        Returns:
            True iiff everything is fine.
        """
        # http://www.python.org/dev/peps/pep-3143/
        ret = False
        try:
            import daemon
            getattr(daemon, "DaemonContext")
            ret = True
        except AttributeError as e:
            # daemon and python-daemon conflict with each other
            log.critical("Please install python-daemon instead of daemon." \
                    "Remove daemon first.")
        except ImportError:
            log.critical("Please install python-daemon.")
        return ret

    #--------------------------------------------------------------------------
    # Initialization
    #--------------------------------------------------------------------------

    def __init__(self, name,
        uid = os.getuid(),
        gid = os.getgid(),
        working_directory = '/',
        debug_mode = False,
        no_daemon = False,
        pid_filename = None
    ):
        self._name = name
        self._uid = uid
        self._gid = gid
        self._working_directory = working_directory
        self._debug_mode = debug_mode
        self._no_daemon = no_daemon
        self._pid_filename = pid_filename if pid_filename \
                 else '/var/run/{}.pid'.format(name)

        # Reference which file descriptors must remain opened while
        # daemonizing (for instance the file descriptor related to
        # the logger, a socket file created before daemonization etc.)
        self._files_to_keep = list()
        self._lock_file = None

        self.initialize()

    #------------------------------------------------------------------------


    def remove_pid_file(self):
        """
        (Internal usage)
        Remove the PID file
        """
        if os.path.exists(self._pid_filename) == True:
            log.info("Removing %s" % self._pid_filename)
            os.remove(self._pid_filename)

        if self._lock_file and self._lock_file.is_locked():
            self._lock_file.release()

    def make_pid_file(self):
        """
        Create a PID file if required in which we store the PID of the daemon
        if needed
        """
        if self._pid_filename and not self._no_daemon:
            atexit.register(self.remove_pid_file)
            open(self._pid_filename, "w+").write("%s\n" % str(os.getpid()))

    def get_pid_from_pid_file(self):
        """
        Retrieve the PID of the daemon thanks to the pid file.
        Returns:
            An integer containing the PID of this daemon.
            None if the pid file is not readable or does not exists
        """
        pid = None
        if self._pid_filename:
            try:
                f_pid = file(self._pid_filename, "r")
                pid = int(f_pid.read().strip())
                f_pid.close()
            except IOError:
                pid = None
        return pid

    def make_lock_file(self):
        """
        Prepare the lock file required to manage the pid file.
        Initialize self.lock_file
        Returns:
            True iif successful.
        """
        if self._pid_filename and not self._no_daemon:
            log.debug("Daemonizing using pid file '%s'" % self._pid_filename)
            self.lock_file = lockfile.FileLock(self._pid_filename)
            if self.lock_file.is_locked() == True:
                log.error("'%s' is already running ('%s' is locked)." % \
                        (self._name, self._pid_filename))
                return False
            self.lock_file.acquire()
        else:
            self.lock_file = None
        return True

    def start(self):
        """
        Start the daemon.
        """
        # Check whether daemon module is properly installed
        if self.check_python_daemon() == False:
            self._terminate()
        import daemon

        # Prepare self.lock_file
        if not self.make_lock_file():
            sys.exit(1)
 
        # We might need to preserve a few files from logging handlers
        files_to_keep = list()
        #for handler in log.handlers:
        #   preserve_files

        if self._no_daemon:
            self.main()
            return

        # Prepare the daemon context
        dcontext = daemon.DaemonContext(
            detach_process    = not self._no_daemon,
            working_directory = self._working_directory,
            pidfile           = self.lock_file,
            stdin             = sys.stdin,
            stdout            = sys.stdout,
            stderr            = sys.stderr,
            uid               = self._uid,
            gid               = self._gid,
            files_preserve    = files_to_keep
        )

        # Prepare signal handling to stop properly if the daemon is killed
        # Note that signal.SIGKILL can't be handled:
        # http://crunchtools.com/unixlinux-signals-101/
        dcontext.signal_map = {
            signal.SIGTERM : self.signal_handler,
            signal.SIGQUIT : self.signal_handler,
            signal.SIGINT  : self.signal_handler
        }

        with dcontext:
            log.info("Entering daemonization")
            self.make_pid_file()

            try:
                self.main()
            except Exception as e:
                log.error("Unhandled exception in start: %s" % e)
                log.error(traceback.format_exc())
            finally:
                self._terminate()

    def signal_handler(self, signal_id, frame):
        """
        (Internal use)
        Stop the daemon (signal handler)
        Args:
            signal_id: The integer identifying the signal
                (see also "man 7 signal")
                Example: 15 if the received signal is signal.SIGTERM
            frame:
        """
        self._terminate()

    def _terminate(self):
        """
        Stops gracefully the daemon.
        Note:
            The lockfile should implicitly released by the daemon package.
        """
        log.info("Stopping %s" % self.__class__.__name__)
        self.terminate()
        self.remove_pid_file()
        self.leave()

    def leave(self):
        """
        Overload this method if you use twisted (see xmlrpc.py)
        """
        os._exit(0)

    # Overload these...

    def initialize(self):
        pass

    def main(self):
        raise NotImplementedError

    def terminate(self):
        pass
