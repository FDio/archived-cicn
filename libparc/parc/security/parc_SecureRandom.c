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

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/security/parc_SecureRandom.h>

struct parc_securerandom {
    int randomfd;
};

static bool
_parcSecureRandom_Destructor(PARCSecureRandom **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCSecureRandom pointer.");
    PARCSecureRandom *instance = *instancePtr;

    close(instance->randomfd);

    return true;
}

parcObject_ImplementAcquire(parcSecureRandom, PARCSecureRandom);
parcObject_ImplementRelease(parcSecureRandom, PARCSecureRandom);
parcObject_Override(PARCSecureRandom, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcSecureRandom_Destructor);

void
parcSecureRandom_AssertValid(const PARCSecureRandom *instance)
{
    parcAssertTrue(parcSecureRandom_IsValid(instance),
               "PARCSecureRandom is not valid.");
}

PARCSecureRandom *
parcSecureRandom_Create()
{
    PARCSecureRandom *result = NULL;

    int fd = open("/dev/urandom", O_RDWR);
    if (fd != -1) {
        result = parcObject_CreateInstance(PARCSecureRandom);
        if (result != NULL) {
            result->randomfd = fd;
        } else {
            close(fd);
        }
    }

    return result;
}

static void
_parcSecureRandom_ReSeed(PARCSecureRandom *random, PARCBuffer *buffer)
{
    size_t length = parcBuffer_Remaining(buffer);
    int wrote_bytes = write(random->randomfd, parcBuffer_Overlay(buffer, length), length);
}

PARCSecureRandom *
parcSecureRandom_CreateWithSeed(PARCBuffer *seed)
{
    PARCSecureRandom *result = parcSecureRandom_Create();

    if (result != NULL) {
        _parcSecureRandom_ReSeed(result, seed);
    }

    return result;
}

uint32_t
parcSecureRandom_Next(PARCSecureRandom *random)
{
    uint32_t value;
    int read_bytes = read(random->randomfd, &value, sizeof(value));
    return value;
}

ssize_t
parcSecureRandom_NextBytes(PARCSecureRandom *random, PARCBuffer *buffer)
{
    size_t length = parcBuffer_Remaining(buffer);
    ssize_t result = read(random->randomfd, parcBuffer_Overlay(buffer, 0), length);
    return result;
}

bool
parcSecureRandom_IsValid(const PARCSecureRandom *instance)
{
    bool result = false;

    if (instance != NULL) {
        if (instance->randomfd != -1) {
            result = true;
        }
    }

    return result;
}
