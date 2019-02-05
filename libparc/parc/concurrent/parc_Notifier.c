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

#ifndef _WIN32
#include <unistd.h>
#endif

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <parc/assert/parc_Assert.h>
#include <parc/concurrent/parc_Notifier.h>
#include <parc/algol/parc_Object.h>

#if defined(__GNUC__)
#define ATOMIC_ADD_AND_FETCH(ptr, increment)      __sync_add_and_fetch(ptr, increment)
#define ATOMIC_BOOL_CAS(ptr, oldvalue, newvalue)  __sync_bool_compare_and_swap(ptr, oldvalue, newvalue)
#define ATOMIC_FETCH(ptr)                         ATOMIC_ADD_AND_FETCH(ptr, 0)
#define ATOMIC_SET(ptr, oldvalue, newvalue)       ATOMIC_BOOL_CAS(ptr, oldvalue, newvalue)
#elif defined(_WIN32)
#define ATOMIC_ADD_AND_FETCH(ptr, increment)      (++(*(ptr)))
#define ATOMIC_BOOL_CAS(ptr, oldvalue, newvalue)  (InterlockedCompareExchangePointer((volatile PVOID *)ptr, (PVOID)newvalue, (PVOID)oldvalue) == ptr)
#define ATOMIC_FETCH(ptr)                         ATOMIC_ADD_AND_FETCH(ptr, 0)
#define ATOMIC_SET(ptr, oldvalue, newvalue)       ATOMIC_BOOL_CAS(ptr, oldvalue, newvalue)
#else
#error "Only GNUC supported, we need atomic operations"
#endif

struct parc_notifier {
    volatile int paused;

    // If the notifications are paused and there is an event,
    // we indicate that we skipped a notify
    volatile int skippedNotify;

#define PARCNotifierWriteFd 1
#define PARCNotifierReadFd 0
    int fds[2];
};

static void
_parcNotifier_Finalize(PARCNotifier **notifierPtr)
{
    PARCNotifier *notifier = *notifierPtr;

    close(notifier->fds[0]);
    close(notifier->fds[1]);
}

parcObject_ExtendPARCObject(PARCNotifier, _parcNotifier_Finalize, NULL, NULL, NULL, NULL, NULL, NULL);

static bool
_parcNotifier_MakeNonblocking(PARCNotifier *notifier)
{

#ifndef _WIN32
    // set the read side to be non-blocking
    int flags = fcntl(notifier->fds[PARCNotifierReadFd], F_GETFL, 0);
    if (flags == 0) {
        if (fcntl(notifier->fds[PARCNotifierReadFd], F_SETFL, flags | O_NONBLOCK) == 0) {
            return true;
        }
    }
    perror("fcntl error");
#endif

    return false;
}

PARCNotifier *
parcNotifier_Create(void)
{
    PARCNotifier *notifier = parcObject_CreateInstance(PARCNotifier);
    if (notifier) {
        notifier->paused = false;
        notifier->skippedNotify = false;

#ifndef _WIN32
        int failure = pipe(notifier->fds);
        parcAssertFalse(failure, "Error on pipe: %s", strerror(errno));

        if (!_parcNotifier_MakeNonblocking(notifier)) {
            parcObject_Release((void **)&notifier);
        }
#endif

    }

    return notifier;
}

parcObject_ImplementAcquire(parcNotifier, PARCNotifier);

parcObject_ImplementRelease(parcNotifier, PARCNotifier);


int
parcNotifier_Socket(PARCNotifier *notifier)
{
    return notifier->fds[PARCNotifierReadFd];
}

bool
parcNotifier_Notify(PARCNotifier *notifier)
{
    if (ATOMIC_BOOL_CAS(&notifier->paused, 0, 1)) {
        // old value was "0" so we need to send a notification
        uint8_t one = 1;
        ssize_t written;
        do {
            written = write(notifier->fds[PARCNotifierWriteFd], &one, 1);
            parcAssertTrue(written >= 0, "Error writing to socket %d: %s", notifier->fds[PARCNotifierWriteFd], strerror(errno));
        } while (written == 0);

        return true;
    } else {
        // we're paused, so count up the pauses
        ATOMIC_ADD_AND_FETCH(&notifier->skippedNotify, 1);
        return false;
    }
}

void
parcNotifier_PauseEvents(PARCNotifier *notifier)
{
    // reset the skipped counter so we count from now until the StartEvents call
    notifier->skippedNotify = 0;
    ATOMIC_BOOL_CAS(&notifier->paused, 0, 1);

    // now clear out the socket
    uint8_t buffer[16];
    while (read(notifier->fds[PARCNotifierReadFd], &buffer, 16) > 0) {
        ;
    }
}

void
parcNotifier_StartEvents(PARCNotifier *notifier)
{
    ATOMIC_BOOL_CAS(&notifier->paused, 1, 0);
    if (notifier->skippedNotify) {
        // we missed some notifications, so re-signal ourself
        parcNotifier_Notify(notifier);
    }
}