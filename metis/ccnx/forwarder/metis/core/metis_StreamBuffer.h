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
 * Wrapper around event scheduler
 */

#ifndef Metis_metis_StreamBuffer_h
#define Metis_metis_StreamBuffer_h

#include <parc/algol/parc_EventQueue.h>
#include <stdbool.h>

void metisStreamBuffer_Destroy(PARCEventQueue **bufferPtr);

/**
 * @function metisStreamBuffer_SetWatermark
 * @abstract Sets the read and/or write watermarks
 * @discussion
 *   For a read watermark, when there is at least <code>low</code> bytes available to read,
 *   the read callback will be fired.  If the bytes in the buffer exceed <code>high</code>,
 *   the stream buffer will stop reading from the network.
 *
 *   For a write watermark, when the bytes in the buffer fall below <code>low</code>, the
 *   write callback is fired.   The <code>high</code> watermark limits stream filters
 *   and shapers from exceeding that threashold on what they write to the buffer.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisStreamBuffer_SetWatermark(PARCEventQueue *buffer, bool setRead, bool setWrite, size_t low, size_t high);

/**
 * @function metisStreamBuffer_Flush
 * @abstract The buffer will read/write more data if available
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return -1 error, 0 no more data, 1 more data
 */
int metisStreamBuffer_Flush(PARCEventQueue *buffer, bool flushRead, bool flushWrite);

/**
 * @function metisStreamBuffer_FlushCheckpoint
 * @abstract Flushes the stream, checkpointing all data in the buffer
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
int metisStreamBuffer_FlushCheckpoint(PARCEventQueue *buffer, bool flushRead, bool flushWrite);

/**
 * @function metisStreamBuffer_FlushFinished
 * @abstract Flush the stream and indicate the end of new data
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
int metisStreamBuffer_FlushFinished(PARCEventQueue *buffer, bool flushRead, bool flushWrite);

/**
 * @typedef MetisStreamBufferReadWriteCallback
 * @abstract Callback when data is available or write space available
 * @constant user_data opaque data passed to <code>MetisStreamBuffer_SetCallbacks()</code>
 * @discussion <#Discussion#>
 */
typedef void (MetisStreamBufferReadWriteCallback)(PARCEventQueue *buffer, void *user_data);

/**
 * @typedef MetisStreamBufferEventCallback
 * @abstract Callback on error or other event on the stream buffer
 * @constant what logical or of METIS_STREAM events.  METIS_STREAM_READING and METIS_STREAM_WRITING
 *                indicate if the error was on the read or write direction.  The conditions
 *                may be METIS_STREAM_ERROR, METIS_STREAM_EOF, METIS_STREAM_TIMEOUT, or METIS_STREAM_CONNECTED.
 * @constant user_data opaque data passed to <code>MetisStreamBuffer_SetCallbacks()</code>
 * @discussion <#Discussion#>
 */
typedef void (MetisStreamBufferEventCallback)(PARCEventQueue *buffer, short what, void *user_data);

/**
 * Changes the callbacks for a buffer event.
 *
 * @param bufev the buffer event object for which to change callbacks
 * @param readcb callback to invoke when there is data to be read, or NULL if
 * no callback is desired
 * @param writecb callback to invoke when the file descriptor is ready for
 * writing, or NULL if no callback is desired
 * @param eventcb callback to invoke when there is an event on the file
 * descriptor
 * @param cbarg an argument that will be supplied to each of the callbacks
 * (readcb, writecb, and errorcb)
 * @see parcEventQueue_Create()
 */
void metisStreamBuffer_SetCallbacks(PARCEventQueue *buffer,
                                    PARCEventQueue_Callback *readCallback,
                                    PARCEventQueue_Callback *writeCallback,
                                    PARCEventQueue_EventCallback *eventCallback,
                                    void *user_data);

/**
 * @function MetisStreamBuffer_EnableCallbacks
 * @abstract Enables specified callbacks.  Does not affect others.
 * @discussion
 *   Enables disabled callbacks.  If a callback is already enabled, has no effect.
 *   A "false" value does not disable it.
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisStreamBuffer_EnableCallbacks(PARCEventQueue *buffer, bool enableRead, bool enableWrite);

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
void metisStreamBuffer_DisableCallbacks(PARCEventQueue *buffer, bool disableRead, bool disableWrite);
#endif // Metis_metis_StreamBuffer_h
