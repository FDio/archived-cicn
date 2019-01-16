/*
 * Copyright (c) 2019 Cisco and/or its affiliates.
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>


#ifdef NDEBUG

#define parcClean_errno() (errno == 0 ? "None" : strerror(errno))
#define parcLog_PrintError(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\nExit (Failure)\n", __FILE__, __LINE__, parcClean_errno(), ##__VA_ARGS__);

#define parcAssertTrue(A, M, ...) if(!(A)) {assert(A); }
#define parcAssertFalse(A, M, ...) if((A)) {assert(!(A)); }
#define parcAssertNotNull(A, M, ...) if(A == NULL) {assert(A != NULL); }
#define parcAssertNull(A, M, ...) if(A != NULL) {assert(A == NULL); }

#define parcTrapIllegalValueIf(A, M, ...) if((A)) {assert(!(A)); }
#define parcTrapIllegalValue(A, M, ...) {parcLog_PrintError("Illegal value: " M, ##__VA_ARGS__); assert(0); exit(1);}
#define parcTrapNotImplemented(M, ...) {parcLog_PrintError("Feature not implemented: " M, ##__VA_ARGS__); assert(0); }
#define parcTrapOutOfBounds(A, M, ...) {parcLog_PrintError("Element out of bounds, %zu :" M, A, ##__VA_ARGS__); assert(0); exit(1);}
#define parcTrapOutOfBoundsIf(A, M, ...) {assert(!(A)); }
#define parcTrapOutOfMemory(M, ...) {parcLog_PrintError("Out of memory. " M, ##__VA_ARGS__); assert(0); exit(1);}
#define parcTrapOutOfMemoryIf(A, M, ...) {assert(!(A)); }
#define parcTrapUnexpectedState(M, ...) {parcLog_PrintError("Unexpected state. " M, ##__VA_ARGS__); assert(0); exit(1);}
#define parcTrapUnexpectedStateIf(A, M, ...) {assert(!(A)); }
#define parcTrapUnrecoverableState(M, ...) {parcLog_PrintError("Unrecoverable State: " M, ##__VA_ARGS__); assert(0); exit(1);}
#define parcTrapInvalidValueIf(A, M, ...) {assert(!(A)); }
#define parcTrapCannotObtainLock(M, ...) {parcLog_PrintError("Cannot obtain lock: " M, ##__VA_ARGS__); assert(0); exit(1);}
#define parcTrapCannotObtainLockIf(A, M, ...) { assert(!(A)); }
#define parcAssertAligned(address, alignment, ...) {assert( ((alignment & (~alignment + 1)) == alignment) ? (((uintptr_t) address) % alignment) == 0 ? 1 : 0 : 0);}

#else

#define parcLog_PrintError(M, ...) fprintf(stderr, "[ERROR] " M "\n", ##__VA_ARGS__);

#define parcAssertTrue(A, M, ...) if(!(A)) { parcLog_PrintError(M, ##__VA_ARGS__); assert(A); }
#define parcAssertFalse(A, M, ...) if((A)) { parcLog_PrintError(M, ##__VA_ARGS__); assert(!(A)); }
#define parcAssertNotNull(A, M, ...) if(A == NULL) {parcLog_PrintError(M, ##__VA_ARGS__); assert(A != NULL); }
#define parcAssertNull(A, M, ...) if(A != NULL) {parcLog_PrintError(M, ##__VA_ARGS__); assert(A == NULL); }

#define parcTrapIllegalValueIf(A, M, ...) if((A)) {parcLog_PrintError("Illegal value: " M, ##__VA_ARGS__); }
#define parcTrapIllegalValue(A, M, ...) parcLog_PrintError("Illegal value: " M, ##__VA_ARGS__); assert(0);
#define parcTrapNotImplemented(M, ...) parcLog_PrintError("Feature not implemented: " M, ##__VA_ARGS__); assert(0);
#define parcTrapOutOfBounds(A, M, ...) parcLog_PrintError("Element out of bounds, %zu :" M, A, ##__VA_ARGS__); assert(0);
#define parcTrapOutOfBoundsIf(A, M, ...) if((A)) {parcLog_PrintError("Out of bounds: " M, ##__VA_ARGS__); }
#define parcTrapOutOfMemory(M, ...) parcLog_PrintError("Out of memory. " M, ##__VA_ARGS__); assert(0);
#define parcTrapOutOfMemoryIf(A, M, ...) if((A)) {parcLog_PrintError("Out of memory. " M, ##__VA_ARGS__); }
#define parcTrapUnexpectedState(M, ...) parcLog_PrintError("Unexpected state. " M, ##__VA_ARGS__); assert(0);
#define parcTrapUnexpectedStateIf(A, M, ...) if((A)) {parcLog_PrintError("Unexpected state: " M, ##__VA_ARGS__); } if((A))
#define parcTrapUnrecoverableState(M, ...) parcLog_PrintError("Unrecoverable State: " M, ##__VA_ARGS__); assert(0);
#define parcTrapInvalidValueIf(A, M, ...) if((A)) {parcLog_PrintError("Invalid value: " M, ##__VA_ARGS__); }
#define parcTrapCannotObtainLock(M, ...) parcLog_PrintError("Cannot obtain lock: " M, ##__VA_ARGS__); assert(0);
#define parcTrapCannotObtainLockIf(A, M, ...) if((A)) {parcLog_PrintError("Cannot obtain lock: " M, ##__VA_ARGS__); }
#define parcAssertAligned(address, alignment, ...) if ((alignment & (~alignment + 1)) == alignment) { if ((((uintptr_t) address) % alignment) != 0)parcLog_PrintError(__VA_ARGS__);} assert((alignment & (~alignment + 1)) == alignment ? (((uintptr_t) address) % alignment) == 0 ? 1 : 0 : 0);
/*    
    
     ((alignment & (~alignment + 1)) == alignment) ? (((uintptr_t) address) % alignment) == 0 ? 1 : 0 : 0);

bool
longBowRuntime_TestAddressIsAligned(const void *address, size_t alignment)
{
    if ((alignment & (~alignment + 1)) == alignment) {
        return (((uintptr_t) address) % alignment) == 0 ? true : false;
    }
    return false;
}*/
#endif
