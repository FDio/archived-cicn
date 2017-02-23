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
#include <stdio.h>

#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/core/metis_StreamBuffer.h>

void
metisStreamBuffer_Destroy(PARCEventQueue **bufferPtr)
{
    assertNotNull(bufferPtr, "Parameter must be non-null double pointer");
    assertNotNull(*bufferPtr, "Parameter must dereference to non-null pointer");
    parcEventQueue_Destroy(bufferPtr);
    *bufferPtr = NULL;
}

void
metisStreamBuffer_SetWatermark(PARCEventQueue *buffer, bool setRead, bool setWrite, size_t low, size_t high)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");

    short flags = 0;
    if (setRead) {
        flags |= PARCEventType_Read;
    }

    if (setWrite) {
        flags |= PARCEventType_Write;
    }

    parcEventQueue_SetWatermark(buffer, flags, low, high);
}

int
metisStreamBuffer_Flush(PARCEventQueue *buffer, bool flushRead, bool flushWrite)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");

    short flags = 0;
    if (flushRead) {
        flags |= PARCEventType_Read;
    }

    if (flushWrite) {
        flags |= PARCEventType_Write;
    }

    return parcEventQueue_Flush(buffer, flags);
}

// NOT USED!!
int
metisStreamBuffer_FlushCheckpoint(PARCEventQueue *buffer, bool flushRead, bool flushWrite)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");

    short flags = 0;
    if (flushRead) {
        flags |= PARCEventType_Read;
    }

    if (flushWrite) {
        flags |= PARCEventType_Write;
    }

    return parcEventQueue_Flush(buffer, flags);
}

// NOT USED!!
int
metisStreamBuffer_FlushFinished(PARCEventQueue *buffer, bool flushRead, bool flushWrite)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");

    short flags = 0;
    if (flushRead) {
        flags |= PARCEventType_Read;
    }

    if (flushWrite) {
        flags |= PARCEventType_Write;
    }

    return parcEventQueue_Flush(buffer, flags);
}

void
metisStreamBuffer_SetCallbacks(PARCEventQueue *buffer,
                               PARCEventQueue_Callback *readCallback,
                               PARCEventQueue_Callback *writeCallback,
                               PARCEventQueue_EventCallback *eventCallback,
                               void *user_data)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");

    parcEventQueue_SetCallbacks(buffer, readCallback, writeCallback, eventCallback, user_data);
}

void
metisStreamBuffer_EnableCallbacks(PARCEventQueue *buffer, bool enableRead, bool enableWrite)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    short flags = 0;
    if (enableRead) {
        flags |= PARCEventType_Read;
    }
    if (enableWrite) {
        flags |= PARCEventType_Write;
    }

    parcEventQueue_Enable(buffer, flags);
}

/**
 * @function MetisStreamBuffer_DisableCallbacks
 * @abstract Disables specified callbacks.  Does not affect others.
 * @discussion
 *   Disables enabled callbacks.  If a callback is already disabled, has no effect.
 *   A "false" value does not enable it.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void
metisStreamBuffer_DisableCallbacks(PARCEventQueue *buffer, bool disableRead, bool disableWrite)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    short flags = 0;
    if (disableRead) {
        flags |= PARCEventType_Read;
    }
    if (disableWrite) {
        flags |= PARCEventType_Write;
    }

    parcEventQueue_Disable(buffer, flags);
}
