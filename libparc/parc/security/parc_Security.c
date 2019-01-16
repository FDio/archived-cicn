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
 *
 * June 9, 2014 add openssl multithreaded support.
 *              See http://www.openssl.org/docs/crypto/threads.html.
 *              We need to implement locking_function() and threadid_func().
 */
#include <config.h>

#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>

#include <parc/assert/parc_Assert.h>

#include <parc/security/parc_Security.h>
#include <parc/algol/parc_Memory.h>

#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#define OPENSSL_THREAD_DEFINES
#include <openssl/opensslconf.h>
#if !defined(OPENSSL_THREADS)
#error OpenSSL must support threads
#endif

// OPENSSL_VERSION_NUMBER = MMNNFFPPS
// MM = major NN = minor FF = fix PP = patch S = 0 (dev), 1-E (betas), F (release)
// This will require 1.0.1e release minimum version
//#if OPENSSL_VERSION_NUMBER < 0x1000105fL
#if OPENSSL_VERSION_NUMBER < 0x0009081fL
#pragma message(OPENSSL_VERSION_TEXT)
#error OpenSSL version must be at least 0.9.8 release
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <openssl/evp.h>
#include <openssl/err.h>

/*
 * OpenSSL requires one lock per thread, so we have a global lock array
 */
static pthread_mutex_t *perThreadLocks;

static volatile bool parcSecurity_initialized = false;
static unsigned long parcSecurity_count = 0;

pthread_mutex_t parcSecurity_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
_lockingCallback(int mode, int type, const char *file __attribute__((unused)), int line __attribute__((unused)))
{
    int error = 0;
    if (mode & CRYPTO_LOCK) {
        error = pthread_mutex_lock(&(perThreadLocks[type]));
    } else {
        error = pthread_mutex_unlock(&(perThreadLocks[type]));
    }

    parcAssertTrue(error == 0, "Error in pthreads: (%d) %s", errno, strerror(errno));
}

#if OPENSSL_VERSION_NUMBER < 0x1000005fL

// for older pre-1.0 versions of openssl
static unsigned long
_getThreadId(void)
{
    // Now, do the "right thing" with it.  Some systems return a pointer to a struct
    // and other systems return a native type.  OpenSSL (1.0 or later) provide two
    // mapping functions to convert these types of representations to CRYPTO_THREADID.
#if defined(__APPLE__)
    uint64_t threadid = 0;
    int error = pthread_threadid_np(pthread_self(), &threadid);
    parcAssertTrue(error == 0, "Error getting threadid");
    return (unsigned long) threadid;
#elif defined(__linux__)
    // linux (at least ubuntu and redhat) uses unsigned long int
    pthread_t threadid = pthread_self();
    return (unsigned long) threadid;
#else
#error Unsupported platform, only __APPLE__ and __linux__ supported
#endif
}

#else

// For new 1.0 or later versions of openssl
static void
_getThreadId(CRYPTO_THREADID *id)
{
    pthread_t threadid = pthread_self();

    // Now, do the "right thing" with it.  Some systems return a pointer to a struct
    // and other systems return a native type.  OpenSSL (1.0 or later) provide two
    // mapping functions to convert these types of representations to CRYPTO_THREADID.

#if defined(__APPLE__)
    // The Apple Mac OS X pthreads uses a pointer to a struct (struct _opaque_pthread_t)
    CRYPTO_THREADID_set_pointer(id, threadid);
#elif defined(__linux__)
    // linux (at least ubuntu and redhat) uses unsigned long int
    CRYPTO_THREADID_set_numeric(id, threadid);
#else
#error Unsupported platform, only __APPLE__ and __linux__ supported
#endif
}
#endif

static void
_initLocks(void)
{
    perThreadLocks = parcMemory_AllocateAndClear(CRYPTO_num_locks() * sizeof(pthread_mutex_t));
    parcAssertNotNull(perThreadLocks, "parcMemory_AllocateAndClear(%zu) returned NULL",
                  CRYPTO_num_locks() * sizeof(pthread_mutex_t));

    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        pthread_mutex_init(&(perThreadLocks[i]), NULL);
    }

#if OPENSSL_VERSION_NUMBER < 0x1000005fL
    // for older pre-1.0 versions
    CRYPTO_set_id_callback(_getThreadId);
#else
    CRYPTO_THREADID_set_callback(_getThreadId);
#endif

    CRYPTO_set_locking_callback(_lockingCallback);
}

static void
_finiLocks(void)
{
    CRYPTO_set_locking_callback(NULL);
    for (int i = 0; i < CRYPTO_num_locks(); i++) {
        pthread_mutex_destroy(&(perThreadLocks[i]));
    }
    parcMemory_Deallocate((void **) &perThreadLocks);
}

void
parcSecurity_AssertIsInitialized(void)
{
    parcTrapUnexpectedStateIf(parcSecurity_IsInitialized() == false, "PARC Security framework is not initialized.  See parcSecurity_Init()");
}

void
parcSecurity_Init(void)
{
    int lockSuccessful = pthread_mutex_lock(&parcSecurity_mutex) == 0;
    parcAssertTrue(lockSuccessful, "Unable to lock the PARC Security framework.");

    if (!parcSecurity_initialized) {
        _initLocks();
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();

        parcSecurity_initialized = true;
    }
    parcSecurity_count++;

    int unlockSuccessful = pthread_mutex_unlock(&parcSecurity_mutex) == 0;
    parcAssertTrue(unlockSuccessful, "Unable to unlock the PARC Security framework.");
}

bool
parcSecurity_IsInitialized(void)
{
    return parcSecurity_initialized;
}

void
parcSecurity_Fini(void)
{
    int lockSuccessful = pthread_mutex_lock(&parcSecurity_mutex) == 0;
    parcAssertTrue(lockSuccessful, "Unable to lock the PARC Security framework.");

    parcSecurity_count--;
    if (parcSecurity_count == 0) {
        EVP_cleanup();
        ERR_free_strings();
        parcSecurity_initialized = false;
        _finiLocks();
    }

    int unlockSuccessful = pthread_mutex_unlock(&parcSecurity_mutex) == 0;
    parcAssertTrue(unlockSuccessful, "Unable to unlock the PARC Security framework.");
}

#pragma GCC diagnostic pop
