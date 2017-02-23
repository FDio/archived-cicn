/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include <parc/algol/parc_FileOutputStream.h>
#include <parc/logging/parc_LogLevel.h>
#include <parc/logging/parc_LogReporterTextStdout.h>
#include <parc/logging/parc_LogReporterFile.h>

#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>
#include <ccnx/forwarder/metis/metis_About.h>

static void
header(void)
{
    printf("%s\n", metisAbout_About());

    printf("            __  __        _    _\n");
    printf("           |  \\/  |  ___ | |_ (_) ___\n");
    printf("           | |\\/| | / _ \\| __|| |/ __|\n");
    printf("           | |  | ||  __/| |_ | |\\__ \\\n");
    printf("           |_|  |_| \\___| \\__||_||___/\n");

    printf("\n");
}

static void
_usage(int exitCode)
{
    printf("Usage: metis_daemon [--port port] [--daemon] [--capacity objectStoreSize] [--log facility=level] [--log-file filename] [--config file]\n");
    printf("\n");
    printf("Metis is the CCNx 1.0 forwarder, which runs on each end system and as a software forwarder\n");
    printf("on intermediate systems.  metis_daemon is the program to launch Metis, either as a console program\n");
    printf("or a background daemon (detatched from console).  Once running, use the program metis_control to\n");
    printf("configure Metis.\n");
    printf("\n");
    printf("The configuration file contains configuration lines as per metis_control\n");
    printf("If logging level or content store capacity is set in the configuraiton file, it overrides the command-line\n");
    printf("When a configuration file is specified, no default listeners on 'port' are setup.  Only 'add listener' lines\n");
    printf("in the configuration file matter.\n");
    printf("\n");
    printf("If no configuration file is specified, metis_daemon will listen on TCP and UDP ports specified by\n");
    printf("the --port flag (or default port).  It will listen on both IPv4 and IPv6 if available.\n");
    printf("\n");
    printf("Options:\n");
    printf("--port            = tcp port for in-bound connections\n");
    printf("--daemon          = start as daemon process\n");
    printf("--objectStoreSize = maximum number of content objects to cache\n");
    printf("--log             = sets a facility to a given log level.  You can have multiple of these.\n");
    printf("                    facilities: all, config, core, io, message, processor\n");
    printf("                    levels: debug, info, notice, warning, error, critical, alert, off\n");
    printf("                    example: metis_daemon --log io=debug --log core=off\n");
    printf("--log-file        = file to write log messages to (required in daemon mode)\n");
    printf("--config           = configuration filename\n");
    printf("\n");
    exit(exitCode);
}

static void
_setLogLevelToLevel(int logLevelArray[MetisLoggerFacility_END], MetisLoggerFacility facility, const char *levelString)
{
    PARCLogLevel level = parcLogLevel_FromString(levelString);

    if (level < PARCLogLevel_All) {
        // we have a good facility and level
        logLevelArray[facility] = level;
    } else {
        printf("Invalid log level string %s\n", levelString);
        _usage(EXIT_FAILURE);
    }
}

/**
 * string: "facility=level"
 * Set the right thing in the logger
 */
static void
_setLogLevel(int logLevelArray[MetisLoggerFacility_END], const char *string)
{
    char *tofree = parcMemory_StringDuplicate(string, strlen(string));
    char *p = tofree;

    char *facilityString = strsep(&p, "=");
    if (facilityString) {
        char *levelString = p;

        if (strcasecmp(facilityString, "all") == 0) {
            for (MetisLoggerFacility facility = 0; facility < MetisLoggerFacility_END; facility++) {
                _setLogLevelToLevel(logLevelArray, facility, levelString);
            }
        } else {
            MetisLoggerFacility facility;
            for (facility = 0; facility < MetisLoggerFacility_END; facility++) {
                if (strcasecmp(facilityString, metisLogger_FacilityString(facility)) == 0) {
                    break;
                }
            }

            if (facility < MetisLoggerFacility_END) {
                _setLogLevelToLevel(logLevelArray, facility, levelString);
            } else {
                printf("Invalid facility string %s\n", facilityString);
                _usage(EXIT_FAILURE);
            }
        }
    }

    parcMemory_Deallocate((void **) &tofree);
}

static void
_daemonize(void)
{
    if (getppid() == 1) {
        // already a daemon
        return;
    }

    int forkReturn = fork();
    trapUnexpectedStateIf(forkReturn < 0, "Fork error");

    if (forkReturn > 0) {
        // parent exits
        exit(EXIT_SUCCESS);
    }

    // Child daemon detaches
    printf("child continuing, pid = %u\n", getpid());

    // get a new process group independent from old parent
    setsid();

    /* close all descriptors */
    for (int i = getdtablesize(); i >= 0; --i) {
        close(i);
    }

    // reset errno because it might be seg to EBADF from the close calls above
    errno = 0;

    // Redirect stdin and stdout and stderr to /dev/null
    const char *devnull = "/dev/null";
    int nullfile = open(devnull, O_RDWR);
    assertTrue(nullfile >= 0, "Error opening file '%s': (%d) %s", devnull, errno, strerror(errno));

    int ret;
    ret = dup(nullfile);
    assertTrue(ret == 1, "Error duping fd 1 got %d file: (%d) %s", ret, errno, strerror(errno));
    ret = dup(nullfile);
    assertTrue(ret == 2, "Error duping fd 2, got %d file: (%d) %s", ret, errno, strerror(errno));

    // metisForwarder will capture signals
}

static MetisLogger *
_createLogfile(const char *logfile)
{
    int logfd = open(logfile, O_WRONLY | O_APPEND | O_CREAT, S_IWUSR | S_IRUSR);
    if (logfd < 0) {
        fprintf(stderr, "Error opening %s for writing: (%d) %s\n", logfile, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    chmod(logfile, S_IRWXU);

    PARCFileOutputStream *fos = parcFileOutputStream_Create(logfd);
    PARCOutputStream *pos = parcFileOutputStream_AsOutputStream(fos);
    PARCLogReporter *reporter = parcLogReporterFile_Create(pos);

    MetisLogger *logger = metisLogger_Create(reporter, parcClock_Wallclock());

    parcOutputStream_Release(&pos);
    parcLogReporter_Release(&reporter);

    return logger;
}

int
main(int argc, const char *argv[])
{
    header();

    uint16_t port = PORT_NUMBER;
    uint16_t configurationPort = 2001;
    bool daemon = false;
    int capacity = -1;
    const char *configFileName = NULL;

    char *logfile = NULL;

    if (argc == 2 && strcasecmp(argv[1], "-h") == 0) {
        _usage(EXIT_SUCCESS);
    }

    int logLevelArray[MetisLoggerFacility_END];
    for (int i = 0; i < MetisLoggerFacility_END; i++) {
        logLevelArray[i] = -1;
    }

    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "--config") == 0) {
                configFileName = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "--port") == 0) {
                port = atoi(argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "--daemon") == 0) {
                daemon = true;
            } else if (strcmp(argv[i], "--capacity") == 0 || strcmp(argv[i], "-c") == 0) {
                capacity = atoi(argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "--log") == 0) {
                _setLogLevel(logLevelArray, argv[i + 1]);
                i++;
            } else if (strcmp(argv[i], "--log-file") == 0) {
                if (logfile) {
                    // error cannot repeat
                    fprintf(stderr, "Cannot specify --log-file more than once\n");
                    _usage(EXIT_FAILURE);
                }

                logfile = parcMemory_StringDuplicate(argv[i + 1], strlen(argv[i + 1]));
                i++;
            } else {
                _usage(EXIT_FAILURE);
            }
        }
    }

    // set restrictive umask, in case we create any files
    umask(027);

    if (daemon && (logfile == NULL)) {
        fprintf(stderr, "Must specify a logfile when running in daemon mode\n");
        _usage(EXIT_FAILURE);
    }

    if (daemon) {
        // inside this call, parent will EXIT_SUCCESS and child will continue
        _daemonize();
    }

    MetisLogger *logger = NULL;
    if (logfile) {
        logger = _createLogfile(logfile);
        parcMemory_Deallocate((void **) &logfile);
    } else {
        PARCLogReporter *stdoutReporter = parcLogReporterTextStdout_Create();
        logger = metisLogger_Create(stdoutReporter, parcClock_Wallclock());
        parcLogReporter_Release(&stdoutReporter);
    }

    for (int i = 0; i < MetisLoggerFacility_END; i++) {
        if (logLevelArray[i] > -1) {
            metisLogger_SetLogLevel(logger, i, logLevelArray[i]);
        }
    }


    // this will update the clock to the tick clock
    MetisForwarder *metis = metisForwarder_Create(logger);

    MetisConfiguration *configuration = metisForwarder_GetConfiguration(metis);

    if (capacity > -1) {
        metisConfiguration_SetObjectStoreSize(configuration, capacity);
    }

    metisConfiguration_StartCLI(configuration, configurationPort);

    if (configFileName) {
        metisForwarder_SetupFromConfigFile(metis, configFileName);
    } else {
        // NULL to not setup AF_UNIX
        metisForwarder_SetupAllListeners(metis, port, NULL);
    }

    MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(metis);

    metisLogger_Log(logger, MetisLoggerFacility_Core, PARCLogLevel_Alert, "daemon", "metis running port %d configuration-port %d", port, configurationPort);

    metisDispatcher_Run(dispatcher);

    metisLogger_Log(logger, MetisLoggerFacility_Core, PARCLogLevel_Alert, "daemon", "metis exiting port %d", port);

    metisForwarder_Destroy(&metis);

    sleep(2);

    metisLogger_Release(&logger);
    return 0;
}
