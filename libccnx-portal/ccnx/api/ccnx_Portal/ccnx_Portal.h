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
 * @file ccnx_Portal.h
 * @brief A low-level API for CCN Interests, Content Objects, and Control messages.
 *
 * \mainpage CCNxPortal
 *  CCNxPortal is a low-level API providing direct access to individual CCNxInterest and CCNxContentObject messages.
 * The API provides very basic access to the "registration" operations for applications to receive CCNxInterest messages and
 * facilities for using different, pre-configured protocol stacks.
 *
 * An application may have many `CCNxPortal` instances, each instance representing a particular protocol stack configuration.
 * Normally an application uses a `CCNxPortalFactory`to create instances rather than creating `CCNxPortal` instances directly.
 * This permits a factory to be setup to provide common attributes and configuration parameters shared by multiple `CCNxPortal`
 * instances.
 *
 * The input/output functions, whether direct like `ccnxPortal_Send` and `ccnxPortal_Receive`,
 * in indirect (such as `ccnxPortal_Listen`), take a parameter that specifiess a timeout behaviour for the function.
 * As a result, an application may use the functions as blocking or non-blocking I/O as needed without having to use
 * multiple `CCNxPortal` instances with different blocking or non-blocking behaviour.
 *
 * Specifying the timeout behaviour consists of providing a pointer to a `CCNxStackTimeout` value,
 * or the value `CCNxStackTimeout_Never`.
 *
 * * `CCNxStackTimeout_Immediate`:
 * The function returns immediately,
 * after first attempting to perform its function provided it can complete without any blocking.
 * For example `ccnxPortal_Receive` will return either the next `CCNxMetaMessage`,
 * if one is waiting, or NULL indicating no message was available.
 * The function `ccnxPortal_Send` will return after first attempting to enqueue its message on the output message queue.
 * If the function would have to wait for space on the output message queue, then it returns indicating failure.
 *
 * * `CCNxStackTimeout_Microseconds()`:
 * Functions will perform their operations blocking only for the maximum time
 * specified.
 *
 * *  `CCNxStackTimeout_Never`:
 * Functions will perform their operations potentially blocking forever.
 *
 */
#ifndef CCNx_Portal_API_ccnx_Portal_h
#define CCNx_Portal_API_ccnx_Portal_h

struct ccnx_portal_status;
/**
 * @brief The status of the CCNx Portal
 */
typedef struct ccnx_portal_status CCNxPortalStatus;

struct ccnx_portal;
/**
 * @brief The CCNx Portal
 */
typedef struct ccnx_portal CCNxPortal;

#include <parc/security/parc_Identity.h>
#include <parc/security/parc_KeyId.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalAttributes.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalFactory.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalStack.h>

#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/ccnx_ContentObject.h>

/**
 * Create a new `CCNxPortal` instance with the given {@link CCNxPortalStack}.
 *
 * @param [in] attributes A pointer to a valid `CCNxPortalAttributes` instance
 * @param [in] stack A pointer to a valid `CCNxPortalStack` instance.
 *
 * @return non-NULL A pointer to a valid `CCNxPortal` instance that must be released via {@link ccnxPortal_Release}.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxPortal *ccnxPortal_Create(const CCNxPortalAttributes *attributes, const CCNxPortalStack *stack);

/**
 * Increase the number of references to a `CCNxPortal`.
 *
 * Note that new `CCNxPortal` is not created,
 * only that the given `CCNxPortal` reference count is incremented.
 * Discard the reference by invoking `parcBuffer_Release`.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 * @return The input `CCNxPortal` pointer.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortal_Create(attributes, stack);
 *
 *     CCNxPortal *portal2 = ccnxPortal_Acquire(portal);
 *
 *     ccnxPortal_Release(&portal);
 *     ccnxPortal_Release(&portal2);
 * }
 * @endcode
 */
CCNxPortal *ccnxPortal_Acquire(const CCNxPortal *portal);

/**
 * Release a previously acquired reference to the specified `CCNxPortal` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] portalPtr A pointer to a pointer to a `CCNxPortal` instance to be released.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see {@link ccnxPortalFactory_CreatePortal}
 * @see {@link ccnxPortal_Create}
 */
void ccnxPortal_Release(CCNxPortal **portalPtr);

/**
 * Return a pointer to the given `CCNxPortal`'s {@link CCNxPortalStatus} instance.
 *
 * A `CCNxPortalStatus` instance is used to extract information about the state of the open
 * portal, e.g., whether or not an error has occurred or EOF has been reached.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 *
 * @return A non-null pointer to a `CCNxPortalStatus` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     CCNxPortalStatus *status = ccnxPortal_GetStatus(portal);
 *
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 */
const CCNxPortalStatus *ccnxPortal_GetStatus(const CCNxPortal *portal);

/**
 * Get the underlying file descriptor for the given `CCNxPortal`.
 *
 * This is the operating systems file descriptor for use with read, write, close, select, poll, ioctl, and so forth.
 *
 * @b Users should expect that this function will be removed in favor
 * of a more general select/poll implementation that works with instances of `CCNxPortal` as well as normal file descriptors.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 *
 * @return The underlying file descriptor for the given `CCNxPortal`.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     int fileDescriptor = ccnxPortal_GetFile(portal);
 *
 *     // use the fileDescriptor as necessary
 *
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 */
int ccnxPortal_GetFileId(const CCNxPortal *portal);

/**
 * Set the attributes for the specified `CCNxPortal` instance.
 *
 * A {@link CCNxPortalAttributes} instance encapsulates information about the logging and blocked
 * state of the `CCNxPortal` instance. These are immutable instances which are not
 * meant to be changed. Rather, they are intended to configure the attributes
 * of other `CCNxPortal` instances.
 *
 * @param [in,out] portal A pointer to a `CCNxPortal` instance.
 * @param [in] attributes A pointer to a `CCNxPortalAttributes` instance that will be used.
 *
 * @return `true` if the attributes were successfully set, `false` otherwise.
 *
 * Example:
 * @code
 * {
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_GetAttributes}
 */
bool ccnxPortal_SetAttributes(CCNxPortal *portal, const CCNxPortalAttributes *attributes);

/**
 * Listen for CCN Interests in the given {@link CCNxName}, i.e., with the given name prefix.
 *
 * If the local CCN router is available, this induces a route update for the given name.
 * Messaging with the local CCN router are governed by the `CCNxPortalFactory` properties named by `CCNxPortalFactory_LocalRouterTimeout`
 *
 * An invocation of the function will return after the time specified by the `CCNxStackTimeout` value,
 * or the function will potentially wait forever if the value is `CCNxStackTimeout_Never`
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 * @param [in] name A `CCNxName` prefix used to filter and accept Interests.
 * @param [in] secondsToLive The number of seconds for this Listen to remain active.
 * @param [in] timeout A pointer to a `CCNxStackTimeout` value, or `CCNxStackTimeout_Never`.
 *
 * @return `true` The operation succeeded.
 * @return `false` The operation failed. See {@link ccnxPortal_GetStatus}.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *     const char *uri = "lci:/PARC";
 *     CCNxName *name = ccnxName_CreateFromCString(reguri);
 *
 *     if (ccnxPortal_Listen(portal, name, 600, CCNxStackTimeout_MicroSeconds(5000))) {
 *         ...
 *     }
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_Ignore}
 * @see {@link ccnxPortalFactory_CreatePortal}
 */
bool ccnxPortal_Listen(CCNxPortal *restrict portal, const CCNxName *restrict name, const time_t secondsToLive, const CCNxStackTimeout *timeout);

/**
 * Stop listening for Interests with the given {@link CCNxName}.
 *
 * An invocation of the function will return after the time specified by the `CCNxStackTimeout` value,
 * or the function will potentially wait forever if the value is `CCNxStackTimeout_Never`
 *
 * @param [in,out] portal A pointer to a `CCNxPortal` instance.
 * @param [in] name A `CCNxName` name to be ignored.
 * @param [in] timeout A pointer to a `CCNxStackTimeout` value, or `CCNxStackTimeout_Never`.
 *
 * @return `true` The operation succeeded
 * @return `false` The operation failed. See {@link CCNxPortalStatus}.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *     const char *uri = "lci:/PARC";
 *     CCNxName *name = ccnxName_CreateFromCString(reguri);
 *
 *     if (ccnxPortal_Listen(portal, name, 86400, CCNxStackTimeout_Never)) {
 *         ...
 *         if (ccnxPortal_Ignore(portal, name, CCNxStackTimeout_Never)) {
 *             ....
 *         }
 *     }
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_Listen}
 * @see {@link CCNxPortalStatus}
 */
bool ccnxPortal_Ignore(CCNxPortal *portal, const CCNxName *name, const CCNxStackTimeout *timeout);

/**
 * Send a {@link CCNxMetaMessage} to the protocol stack.
 *
 * The portal message may be an Interest, Content Object, or Control Message.
 * The exact type wrapped by the portal message may be determined via
 * {@link ccnxMetaMessage_IsInterest}, {@link ccnxMetaMessage_IsContentObject}, and
 * {@link ccnxMetaMessage_IsControl}. This enables a seamless API for both
 * both producer and consumer applications.
 *
 * An invocation of the function will wait for the time specified by the pointer to the `CCNxStackTimeout` value,
 * or the function will potentially wait forever if the value is `CCNxStackTimeout_Never`.
 *
 * @param [in,out] portal A pointer to a `CCNxPortal` instance.
 * @param [in] message A pointer to a `CCNxMetaMessage` instance.
 * @param [in] timeout A pointer to a `CCNxStackTimeout` value, or `CCNxStackTimeout_Never`.
 *
 * @return `true` No errors occurred.
 * @return `false` A protocol stack error occurred while writing the message (see `ccnxPortal_GetError`).
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);
 *
 *     if (ccnxPortal_Send(portal, message, CCNxStackTimeout_Never)) {
 *         ...
 *     }
 *
 *     ccnxMetaMessage_Release(&message);
 *     ccnxInterest_Release(&interest);
 *     ccnxName_Release(&name);
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_Receive}
 */
bool ccnxPortal_Send(CCNxPortal *restrict portal, const CCNxMetaMessage *restrict message, const CCNxStackTimeout *timeout);

/**
 * Read data from the protocol stack and construct a {@link CCNxMetaMessage}.
 *
 * The portal message may be an Interest, Content Object, or Control Message.
 * The exact type wrapped by the portal message may be determined via
 * {@link ccnxMetaMessage_IsInterest}, {@link ccnxMetaMessage_IsContentObject}, and
 * {@link ccnxMetaMessage_IsControl}. This enables a seamless API for both
 * both producer and consumer applications.
 *
 * An invocation of the function will wait for the time specified by the pointer to the `CCNxStackTimeout` value,
 * or the function will potentially wait forever if the value is `CCNxStackTimeout_Never`.
 *
 * If NULL is returned, the caller may test the value of `errno` to discriminate the conditions.
 *
 * @param [in,out] portal A pointer to a `CCNxPortal` instance.
 * @param [in] timeout A pointer to a `CCNxStackTimeout` value, or `CCNxStackTimeout_Never`.
 *
 * @return CCNxMetaMessage A `CCNxMetaMessage` instance that must be freed via {@link ccnxMetaMessage_Release}.
 * @return NULL An error occurred while reading from the protocol stack (see `ccnxPortal_GetError`).
 *
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/Hello/World");
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(name);
 *
 *     CCNxMetaMessage *message = ccnxMetaMessage_CreateFromInterest(interest);
 *
 *     if (ccnxPortal_Send(portal, message, CCNxStackTimeout_Never)) {
 *         for (int responses = 0; responses == 0;) {
 *             CCNxMetaMessage *message = ccnxPortal_Receive(portal, CCNxStackTimeout_Never);
 *             if (message != NULL) {
 *                 ...
 *             }
 *         }
 *     }
 *
 *     ccnxMetaMessage_Release(&message);
 *     ccnxInterest_Release(&interest);
 *     ccnxName_Release(&name);
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_Send}
 * @see `ccnxMetaMessage_IsInterest`
 * @see `ccnxMetaMessage_IsContentObject`
 * @see `ccnxMetaMessage_IsControl`
 */
CCNxMetaMessage *ccnxPortal_Receive(CCNxPortal *portal, const CCNxStackTimeout *timeout);

/**
 * Get the {@link PARCKeyId} of the identity bound to the given `CCNxPortal` instance.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 *
 * @return The `PARCKeyId` instance associated with this `CCNxPortal`.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal =
 *         ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     PARCKeyId *keyId = ccnxPortal_GetKeyId(portal);
 *
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see ccnxPortalFactory_CreatePortal
 */
const PARCKeyId *ccnxPortal_GetKeyId(const CCNxPortal *portal);

/**
 * Return `true` if the last operation induced an end-of-file state.
 *
 * <b>Currently this is inoperable.  It's likely that the chunked mode of Portal will be deprecated and replaced at a high archtectural level.</b>
 *
 * This only applies to Portal instances configured for Chunked protocol.
 * If the received chunk is equal to the currently `last chunk` this will return true.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 *
 * @return `true` The last operation induced an end-of-file state.
 * @return `false` The last operation did not induce an end-of-file state.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Chunked);
 *
 *     const char *uri = "lci:/PARC";
 *     CCNxName *name = ccnxName_CreateFromCString(reguri);
 *
 *     if (ccnxPortal_Listen(portal, name, 86400, CCNxStackTimeout_Never)) {
 *         ...
 *
 *         bool isEOF = ccnxPortal_IsEOF(portal);
 *         ...
 *     }
 *
 *     ccnxName_Release(&name);
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 */
bool ccnxPortal_IsEOF(const CCNxPortal *portal);

/**
 * Return `true` if the last operation induced an error, `false` otherwise.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 *
 * @return `true` The last operation induced an error.
 * @return `false` The last operation induced no error.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     const char *uri = "lci:/PARC";
 *     CCNxName *name = ccnxName_CreateFromCString(reguri);
 *
 *     if (ccnxPortal_Listen(portal, name, 86400, CCNxStackTimeout_Never)) {
 *         ...
 *
 *         bool isError = ccnxPortal_IsError(portal);
 *         ...
 *     }
 *
 *     ccnxName_Release(&name);
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_GetError}
 */
bool ccnxPortal_IsError(const CCNxPortal *portal);

/**
 * Determine the type of error, if any, that has occurred.
 *
 * The return value corresponds to the values of errno.
 *
 * @param [in] portal A pointer to a `CCNxPortal` instance.
 *
 * @return An integer error code.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_LoopBack);
 *
 *     const char *uri = "lci:/PARC";
 *     CCNxName *name = ccnxName_CreateFromCString(reguri);
 *
 *     if (ccnxPortal_Listen(portal, name, 86400, CCNxStackTimeout_Never)) {
 *         ...
 *
 *         bool isError = ccnxPortal_IsError(portal);
 *         if (isError) {
 *             printf("Error code: %d\n", ccnxPortal_GetError(portal));
 *         }
 *
 *         ...
 *     }
 *
 *     ccnxName_Release(&name);
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_IsError}
 */
int ccnxPortal_GetError(const CCNxPortal *portal);

/**
 * Flush the input and output paths and pause the protocol stack.
 *
 * The timeout value is currently not used, instead this function will block until the operation is complete.
 *
 * @param [in] portal A pointer to a valid instance of `CCNxPortal`.
 * @param [in] timeout A pointer to a `CCNxStackTimeout` value, or `CCNxStackTimeout_Never`.
 *
 * @return `true` If successful.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxPortal_Flush(CCNxPortal *portal, const CCNxStackTimeout *timeout);

#endif  // CCNx_Portal_API_ccnx_Portal_h
