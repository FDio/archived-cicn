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

/**
 */
#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <LongBow/longBow_SubProcess.h>
#include <LongBow/private/longBow_Memory.h>

struct longbow_subprocess {
    const char *path;
    char **arguments;
    pid_t pid;
    int exitStatus;
    struct rusage rusage;
};

static void
_resetAllSignals(void)
{
    struct sigaction signalAction;
    signalAction.sa_handler = SIG_DFL;
    signalAction.sa_flags = SA_SIGINFO;
    sigemptyset(&signalAction.sa_mask);

    for (int i = 1; i < NSIG; i++) {
        sigaction(i, &signalAction, NULL);
    }
}

LongBowSubProcess *
longBowSubProcess_Exec(const char *path, ... /*, (char *)0 */)
{
    va_list ap;
    va_start(ap, path);

    size_t count = 0;
    for (char *arg = va_arg(ap, char *); arg != NULL; arg = va_arg(ap, char *)) {
        count++;
    }

    LongBowSubProcess *result = longBowMemory_Allocate(sizeof(LongBowSubProcess));

    result->path = path;
    longBowMemory_Allocate(sizeof(char *) * count + 1);
    va_end(ap);
    // This relies on the last element being all zeros.
    va_start(ap, path);

    for (size_t i = 0; i < count; i++) {
        result->arguments[i] = va_arg(ap, char *);
    }
    va_end(ap);

    result->pid = fork();
    if (result->pid == 0) {
        _resetAllSignals();
        printf("Exec %s\n", result->path);
        int error = execv(result->path, result->arguments);
        printf("Error %d\n", error);
        perror(path);
        exit(1);
    }

    printf("Process group child=%d, parent=%d\n", getpgid(result->pid), getpgrp());
    return result;
}

void
longBowSubProcess_Destroy(LongBowSubProcess **processPtr)
{
    LongBowSubProcess *process = *processPtr;

    if (process->pid) {
        longBowSubProcess_Signal(process, SIGKILL);
    }

    longBowMemory_Deallocate((void **) process->arguments);
    longBowMemory_Deallocate((void **) processPtr);
}

bool
longBowSubProcess_Terminate(LongBowSubProcess *subProcess)
{
    return longBowSubProcess_Signal(subProcess, SIGTERM);
}

bool
longBowSubProcess_Signal(LongBowSubProcess *subProcess, int signalNumber)
{
    return kill(subProcess->pid, signalNumber) == 0;
}

void
longBowSubProcess_Wait(LongBowSubProcess *subProcess)
{
    wait4(subProcess->pid, &subProcess->exitStatus, 0, &subProcess->rusage);
    subProcess->pid = 0;
}

void
longBowSubProcess_Display(const LongBowSubProcess *subprocess, int indentation)
{
    printf("%*s%s: ", indentation, "", subprocess->path);
    if (subprocess->pid == 0) {
        printf("not running .exitStatus=%d ", subprocess->exitStatus);
    } else {
        printf(".pid=%d", subprocess->pid);
    }

    printf("\n");
}
