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
 * @file parc_EventQueue.h
 * @ingroup events
 * @brief Queue buffer events
 *
 * Provides a facade implementing many regularly available event functions.
 * This is an interface that software implementors may use to substitute
 * different kinds of underlying implementations of these event management functions.
 * Notable examples are libevent and libev.
 *
 */
#ifndef libparc_parc_EventQueue_h
#define libparc_parc_EventQueue_h

#ifndef _WIN32
#include <sys/socket.h>
#endif

#include <sys/types.h>

#include <parc/algol/parc_Event.h>

/**
 * Current implementation based on top of libevent2
 */

/**
 * @typedef PARCEventQueue
 * @brief A structure containing private libevent state data variables
 */
typedef struct PARCEventQueue PARCEventQueue;

/**
 * @typedef PARCEventQueuePair
 * @brief A structure containing private libevent state data for connected queue pairs
 */
typedef struct PARCEventQueuePair PARCEventQueuePair;

/**
 * @typedef PARCEventQueueEventType
 * @brief An enumeration of queue event types
 */
typedef enum {
    PARCEventQueueEventType_Reading = 0x01,
    PARCEventQueueEventType_Writing = 0x02,
    PARCEventQueueEventType_EOF = 0x10,
    PARCEventQueueEventType_Error = 0x20,
    PARCEventQueueEventType_Timeout = 0x40,
    PARCEventQueueEventType_Connected = 0x80
} PARCEventQueueEventType;

/**
 * @typedef PARCEventQueue_Options
 * @brief A structure queue flags
 */
typedef enum {
    PARCEventQueueOption_CloseOnFree = 0x01,
    PARCEventQueueOption_DeferCallbacks = 0x04
} PARCEventQueueOption;

/**
 * @typedef PARCEventQueue_Callback
 * @brief A definition for callback function arguments
 */
typedef void (PARCEventQueue_Callback)(PARCEventQueue *event, PARCEventType type, void *user_data);

/**
 * @typedef PARCEventQueue_EventCallback
 * @brief A definition for callback function arguments
 */
typedef void (PARCEventQueue_EventCallback)(PARCEventQueue *event, PARCEventQueueEventType type, void *user_data);

/**
 * Create a buffer event handler instance.
 *
 * The event instance is passed in. Options can be either, both or none of the following.
 *
 * PARCEventQueue_CloseOnFree
 *     The underlying file descriptor is closed when this event is freed.
 *
 * PARCEventQueue_DeferCallbacks
 *     Callbacks are run deferred in the scheduler.
 *
 * @param [in] eventScheduler - The scheduler instance base.
 * @param [in] fd the file descriptor to monitor
 * @param [in] options as described in flags above
 * @returns A pointer to the a PARCEventQueue instance.
 *
 * Example:
 * @code
 * {
 *     PARCEventQueue *event = parcEventQueue_Create(eventScheduler, fd, PARCEventQueue_CloseOnFree);
 * }
 * @endcode
 *
 */
PARCEventQueue *parcEventQueue_Create(PARCEventScheduler *eventScheduler, int fd, PARCEventQueueOption options);

/**
 * Destroy a buffer event handler instance.
 *
 * The event instance is passed in.
 *
 * @param [in] eventQueue - The address of the instance to destroy.
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_Destroy(&eventQueue);
 * }
 * @endcode
 *
 */
void parcEventQueue_Destroy(PARCEventQueue **eventQueue);

/**
 * Enable events on an instance.
 *
 * The event instance is passed in.
 *
 * @param [in] event - queue instance to enable.
 * @param [in] types - the event(s) to enable.
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_Enable(bufferEvent, PARCEvent_ReadEvent);
 * }
 * @endcode
 *
 */
void parcEventQueue_Enable(PARCEventQueue *event, PARCEventType types);

/**
 * Get enable events on an instance.
 *
 * The event instance is passed in.
 *
 * @param [in] queue - the instance to return enabled events from
 * @returns mask of events which are enabled.
 *
 * Example:
 * @code
 * {
 *     PARCEventType *events = parcEventQueue_GetEnable(queue);
 * }
 * @endcode
 *
 */
PARCEventType parcEventQueue_GetEnabled(PARCEventQueue *queue);

/**
 * Disable events on an instance.
 *
 * The event instance is passed in.
 *
 * @param [in] queue - instance to disable event on.
 * @param [in] types - the events to disable.
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_Disable(queue, types);
 * }
 * @endcode
 *
 */
void parcEventQueue_Disable(PARCEventQueue *queue, PARCEventType types);

/**
 * Set callbacks on a buffer event instance.
 *
 * The event instance is passed in.
 * You can disable a callback by passing NULL instead of the callback function.
 * NB: all the callback functions on a bufferevent share a single user_data
 * value, so changing user_data will affect all of them.
 *
 * @param [in] eventInstance - event instance
 * @param [in] readCallback - callback for read events
 * @param [in] writeCallback - callback for write events
 * @param [in] eventCallback - callback for non-read/write events
 * @param [in] user_data - data passed along in callback arguments
 *
 * Example:
 * @code
 * void Callback(PARCEventType type, void *user_data)
 * {
 *     printf("Received event of type=%d\n", type);
 * }
 * ...
 * {
 * ...
 *     parcEventQueue_SetCallbacks(eventInstance, Callback, NULL, NULL, user_data);
 *     parcEventQueue_Enable(eventInstance, PARCEventType_Read);
 * }
 * @endcode
 *
 */
void parcEventQueue_SetCallbacks(PARCEventQueue *eventInstance,
                                 PARCEventQueue_Callback *readCallback,
                                 PARCEventQueue_Callback *writeCallback,
                                 PARCEventQueue_EventCallback *eventCallback,
                                 void *user_data);

/**
 * Flush events on a queue
 *
 * @param [in] queue instance to flush
 * @param [in] types the type(s) of events to flush
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventQueue_Flush(queue, PARCEventType_Write);
 * }
 * @endcode
 *
 */
int parcEventQueue_Flush(PARCEventQueue *queue, PARCEventType types);

/**
 * Finialized flush of events on a queue
 *
 * @param [in] queue instance to flush
 * @param [in] types the type(s) of events to flush
 * @returns 0 if no data was flushed, 1 if some data was flushed, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventQueue_Finished(queue, PARCEventType_Write);
 * }
 * @endcode
 *
 */
int parcEventQueue_Finished(PARCEventQueue *queue, PARCEventType types);

/**
 * Set watermark boundries on a queue
 *
 * @param [in] queue - queue instance to set watermark on
 * @param [in] types - the events to set watermark on
 * @param [in] low - the low watermark value
 * @param [in] high - the high watermark value
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_SetWatermark(queue, PARCEventType_Read, 0, MAXPATHLEN);
 * }
 * @endcode
 *
 */
void parcEventQueue_SetWatermark(PARCEventQueue *queue, PARCEventType types, size_t low, size_t high);

/**
 * Add formatted text to the end of a queue
 *
 * @param [in] queue - queue instance to write to
 * @param [in] fmt - printf arguments
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_Printf(queue, "%s\n", "Hello world.");
 * }
 * @endcode
 *
 */
int parcEventQueue_Printf(PARCEventQueue *queue, const char *fmt, ...);

/**
 * Set the associated file descriptor on a queue
 *
 * @param [in] queue instance set to monitor this descriptor
 * @param [in] fd file descriptor
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventQueue_SetFileDescriptor(queue, STDIN_FILENO);
 * }
 * @endcode
 *
 */
int parcEventQueue_SetFileDescriptor(PARCEventQueue *queue, int fd);

/**
 * Get the associated file descriptor on a queue
 *
 * @param [in] queue instance set to monitor this descriptor
 * @returns file descriptor on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int fileDescriptor = parcEventQueue_GetFileDescriptor(queue);
 * }
 * @endcode
 *
 */
int parcEventQueue_GetFileDescriptor(PARCEventQueue *queue);

/**
 * Read data from the queue output
 *
 * @param [in] queue instance to read from
 * @param [in] data to read into
 * @param [in] dataLength length of data to read
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventQueue_Read(queue, data, length);
 * }
 * @endcode
 *
 */
int parcEventQueue_Read(PARCEventQueue *queue, void *data, size_t dataLength);

/**
 * Add data to the queue output
 *
 * @param [in] queue instance to add to
 * @param [in] data to write
 * @param [in] dataLength length of data to write
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     int result = parcEventQueue_Write(queue, data, length);
 * }
 * @endcode
 *
 */
int parcEventQueue_Write(PARCEventQueue *queue, void *data, size_t dataLength);

/**
 * Attach an launch a socket on a queue
 *
 * @param [in] queue instance to attach socket to
 * @param [in] address socket data
 * @param [in] addressLength socket data length
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_un addr_unix;
 *     memset(&addr_unix, 0, sizeof(addr_unix));
 *     addr_unix.sun_family = AF_UNIX;
 *     strcpy(addr_unix.sun_path, sock_name);
 *     int result = parcEventQueue_ConnectSocket(queue, addr_unix, sizeof(addr_unix));
 * }
 * @endcode
 *
 */
int parcEventQueue_ConnectSocket(PARCEventQueue *queue, struct sockaddr *address, int addressLength);

/**
 * Set queue priority
 *
 * @param [in] queue instance to modify
 * @param [in] priority queue priority
 * @returns 0 on success, -1 on failure
 *
 * Example:
 * @code
 * {
 *     parcEvent_Enable(queue, PARCEventQueuePriority_Normal);
 * }
 * @endcode
 *
 */
int parcEventQueue_SetPriority(PARCEventQueue *queue, PARCEventPriority priority);

/**
 * Create a pair of connected queues
 *
 * @param [in] eventScheduler event scheduler instance
 * @returns a queue pair instance
 *
 * Example:
 * @code
 * {
 *     PARCEventQueuePair *pair = parcEventQueue_CreateConnectedPair(eventScheduler);
 * }
 * @endcode
 *
 */
PARCEventQueuePair *parcEventQueue_CreateConnectedPair(PARCEventScheduler *eventScheduler);

/**
 * Destroy a connected queue pair
 *
 * @param [in] queuePair queue pair instance address to destroy
 *
 * Example:
 * @code
 * {
 *     parcEventQueuePairCreateparcEventQueue_DestroyConnectedPair(&queuePair);
 * }
 * @endcode
 *
 */
void parcEventQueue_DestroyConnectedPair(PARCEventQueuePair **queuePair);

/**
 * Return the downward queue of a pair
 *
 * @param [in] queuePair queue pair instance address to destroy
 *
 * Example:
 * @code
 * {
 *     PARCEventQueue *downQueue = parcEventQueue_GetConnectedDownQueue(queuePair);
 * }
 * @endcode
 *
 */
PARCEventQueue *parcEventQueue_GetConnectedDownQueue(PARCEventQueuePair *queuePair);

/**
 * Return the upward queue of a pair
 *
 * @param [in] queuePair queue pair instance address to destroy
 *
 * Example:
 * @code
 * {
 *     PARCEventQueue *upQueue = parcEventQueue_GetConnectedUpQueue(queuePair);
 * }
 * @endcode
 *
 */
PARCEventQueue *parcEventQueue_GetConnectedUpQueue(PARCEventQueuePair *queuePair);

/**
 * Private Internal Function - return internal input buffer of a queue
 *
 * The event instance is passed in.
 *
 * @param [in] queue the event queue
 * @returns private evbuffer pointer
 *
 * Example:
 * @code
 * {
 *     struct evbuffer *evbuffer = internal_parcEventQueue_GetEvInputBuffer(queue);
 * }
 * @endcode
 *
 */
struct evbuffer *internal_parcEventQueue_GetEvInputBuffer(PARCEventQueue *queue);

/**
 * Private Internal Function - return internal output buffer of a queue
 *
 * The event instance is passed in.
 *
 * @param [in] queue the event queue
 * @returns private evbuffer pointer
 *
 * Example:
 * @code
 * {
 *     struct evbuffer *evbuffer = internal_parcEventQueue_GetEvOutputBuffer(queue);
 * }
 * @endcode
 *
 */
struct evbuffer *internal_parcEventQueue_GetEvOutputBuffer(PARCEventQueue *queue);

/**
 * Turn on debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_EnableDebug();
 * }
 * @endcode
 *
 */
void parcEventQueue_EnableDebug(void);

/**
 * Turn off debugging flags and messages
 *
 * Example:
 * @code
 * {
 *     parcEventQueue_DisableDebug();
 * }
 * @endcode
 *
 */
void parcEventQueue_DisableDebug(void);
#endif // libparc_parc_EventQueue_h
