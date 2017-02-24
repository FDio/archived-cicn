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
 * @file ccnx_Control.h
 * @brief This is a stack control message.
 *
 * This may induce other Control messages for the stack, for the Forwarder, or potentially
 * for the network.
 *
 */

#ifndef libccnx_ccnx_Control_h
#define libccnx_ccnx_Control_h

#include <stdint.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

#include <ccnx/api/notify/notify_Status.h>

#include <ccnx/api/control/cpi_RouteEntry.h>
#include <ccnx/api/control/cpi_ForwardingStrategy.h>
#include <ccnx/api/control/cpi_ManageWldr.h>
#include <ccnx/api/control/cpi_InterfaceIPTunnel.h>

/**
 * @typedef CCNxControl
 * @brief Control message for CCNx.
 */
typedef CCNxTlvDictionary CCNxControl;

/**
 * Increase the number of references to a `CCNxControl` instance.
 *
 * Note that new `CCNxControl` is not created,
 * only that the given `CCNxControl` reference count is incremented.
 * Discard the reference by invoking {@link ccnxControl_Release()}.
 *
 * @param [in] control A pointer to the original instance.
 * @return The value of the input parameter @p control.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxControl *control = ccnxControl_Acquire(instance);
 *
 *     ccnxControl_Release(&control);
 *
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_Acquire(const CCNxControl *control);

/**
 * Print a human readable representation of the given `CCNxControl` instance.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] control A pointer to the instance to display.
 *
 * Example:
 * @code
 * {
 *     CCNxControl *control = < ... >
 *
 *     ccnxControl_Display(control, 4);
 *
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 */
void ccnxControl_Display(const CCNxControl *control, int indentation);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] controlP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxControl *control = < ... >
 *
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Acquire}
 */
void ccnxControl_Release(CCNxControl **controlP);

/**
 * Return the original sequence number to which an ACK corresponds.
 *
 * Control plane messages contain sequence numbers. When an ACK is received, this function
 * returns the sequence number of the control plane message being ACKed.
 *
 * @param [in] control A pointer to a `CCNxControl` instance.
 *
 * @return The sequence number of the control plane message being ACKed.
 *
 * Example:
 * @code
 * {
 *     uint64_t originalSequenceNumber = ccnxControl_GetAckOriginalSequenceNumber(control);
 * }
 * @endcode
 */
uint64_t ccnxControl_GetAckOriginalSequenceNumber(const CCNxControl *control);

/**
 * Return true if the specified `CCNxControl` instance is a Notification.
 *
 * @param [in] control A pointer to a `CCNxControl` instance.
 *
 * @return `true` if the specified `CCNxControl` instance is a Notification message.
 * @return `false` if the specified `CCNxControl` instance is not a Notification message.
 *
 * Example:
 * @code
 * {
 *     bool isNotification = ccnxControl_IsNotification(control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_IsACK}
 */
bool ccnxControl_IsNotification(const CCNxControl *control);

/**
 * Return `true` if the specified `CCNxControl` instance is an ACK message carrying an ACK (not a NACK)
 *
 * An acknolwedgement message can be either a positive (ACK) or negative (NACK) acknowlegement.
 * In both cases, it carries the original sequence number of the message being ACKed or NACKed.
 *
 * @param [in] control A pointer to a `CCNxControl` instance.
 *
 * @return `true` if the specified `CCNxControl` instance is an Ack message.
 * @return `false` if the specified `CCNxControl` instance is not an Ack message.
 *
 * Example:
 * @code
 * {
 *     bool isAck = ccnxControl_IsACK(control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_IsNotification}
 */
bool ccnxControl_IsACK(const CCNxControl *control);

/**
 * Return `true` if the specified `CCNxControl` instance is an ACK message carrying a NACK (not a ACK)
 *
 * An acknolwedgement message can be either a positive (ACK) or negative (NACK) acknowlegement.
 * In both cases, it carries the original sequence number of the message being ACKed or NACKed.
 *
 * @param [in] control A pointer to a `CCNxControl` instance.
 *
 * @return `true` if the specified `CCNxControl` is an NACK.
 * @return `false` if the specified `CCNxControl` instance is not an NAck message.
 *
 * Example:
 * @code
 * {
 *     bool isAck = ccnxControl_IsACK(control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_IsNotification}
 */
bool ccnxControl_IsNACK(const CCNxControl *control);

/**
 * Get the {@link NotifyStatus} from a `CCNxControl` instance, if it exists.
 *
 * This function creates a new instance of `NotifyStatus`, initialized from the specified
 * `CCNxControl`, which must eventually be released by calling {@link notifyStatus_Release}().
 * If the specified `CCNxControl` instance does not contain a `NotifyStatus`, this function will return NULL.
 *
 * @param [in] control A pointer to the instance of `CCNxControl` from which to retrieve the `NotifyStatus`.
 *
 * @return An instance of `NotifyStatus`, if it exists.
 * @return NULL If the `CCNxControl` instance did not contain a `NotifyStatus`.
 *
 * Example:
 * @code
 * {
 *     NotifyStatus status = ccnxControl_GetNotifyStatus(control);
 *
 *     notifyStatus_Release(&status);
 * }
 * @endcode
 *
 * @see {@link notifyStatus_Release}
 * @see {@link NotifyStatus}
 */
NotifyStatus *ccnxControl_GetNotifyStatus(const CCNxControl *control);

/**
 * Create a new `CCNxControl` instance containing a request to add a route to the control plane.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release()}.
 *
 * @param [in] route The {@link CPIRouteEntry} to add.
 *
 * @return A new `CCNxControl` instance encapsulating a request to add the specified route.
 *
 * Example:
 * @code
 * {
 *     CPIRouteEntry *cpiRouteEntry = cpiRouteEntry_Create(...);
 *
 *     CCNxControl *control = ccnxControl_CreateAddRouteRequest(cpiRouteEntry);
 *
 *     cpiRouteEntry_Destroy(&cpiRouteEntry);
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_CreateRemoveRouteRequest}
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateAddRouteRequest(const CPIRouteEntry *route);

/**
 * Create a new `CCNxControl` instance containing a request to remove a route from the control plane.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @param [in] route The {@link CPIRouteEntry} to remove.
 *
 * @return A new `CCNxControl` instance encapsulating a request to remove the specified route.
 *
 * Example:
 * @code
 * {
 * {
 *     CPIRouteEntry *cpiRouteEntry = cpiRouteEntry_Create(...);
 *
 *     CCNxControl *control = ccnxControl_CreateRemoveRouteRequest(cpiRouteEntry);
 *
 *     cpiRouteEntry_Destroy(&cpiRouteEntry);
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_CreateAddRouteRequest}
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateRemoveRouteRequest(const CPIRouteEntry *route);

CCNxControl *ccnxControl_CreateSetStrategyRequest(const CPIForwardingStrategy *fwdStrategy);

CCNxControl *ccnxControl_CreateSetWldrRequest(const CPIManageWldr *cpiWldr);

/**
 * Create a new `CCNxControl` instance containing a request to add a route for CCN messages matching the given {@link CCNxName}
 * back to the caller's network interface.
 *
 * The created `CCNxControl` message describes to the forwarder that messages matching the specified `CCNxName` should be
 * routed back to the caller. This is how to initiate listening for a name.
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @param [in] name A pointer to a `CCNxName` instance containing the name to match against.
 *
 * @return A new `CCNxControl` instance encapsulating a request to add a route for the given `CCNxName` back to the caller's
 *         network interface.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/thingie");
 *
 *     CCNxControl *control = ccnxControl_CreateAddRouteToSelfRequest(name);
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_CreateRemoveRouteToSelfRequest}
 * @see {@link ccnxControl_CreateAddRouteRequest}
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateAddRouteToSelfRequest(const CCNxName *name);

/**
 * Create a new `CCNxControl` instance containing a request to remove a route to the caller for messages matching the specified
 * {@link CCNxName}.
 *
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @param [in] name A pointer to a `CCNxName` instance containing the name to match against.
 *
 * @return A new `CCNxControl` instance encapsulating a request to remove the specified route.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/parc/csl/media/thingie");
 *
 *     CCNxControl *control = ccnxControl_CreateRemoveRouteToSelfRequest(name);
 *     ...
 *
 *     ccnxName_Release(&name);
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_CreateAddRouteToSelfRequest}
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateRemoveRouteToSelfRequest(const CCNxName *name);

/**
 * Create a new `CCNxControl` instance containing the specified CPI command, and including the
 * flag indicating that it is a CPI message.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @param [in] json A pointer to a {@link PARCJSON} instance containing CPI command to wrap.
 *
 * @return A new `CCNxControl` instance containing the specified CPI command.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
 *     PARCJSON *cpiRequest = cpiCancelFlow_CreateRequest(name);
 *     CCNxControl *control = ccnxControl_CreateCPIRequest(cpiRequest);
 *
 *     ...
 *
 *     parcJSON_Release(&cpiRequest);
 *     ccnxControl_Release(&control);
 *     ccnxName_Release(&name);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateCPIRequest(PARCJSON *json);

/**
 * Create a new `CCNxControl` instance containing a "List Routes" request.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @return A new `CCNxControl` instance containing the request.
 *
 * Example:
 * @code
 * {
 *     CCNxControl *control = ccnxControl_CreateRouteListRequest();
 *     PARCJSON *json = ccnxControl_GetJson(control);
 *
 *     ...
 *
 *     ccnxControl_Release(&control);
 *  }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateRouteListRequest(void);

/**
 * Create a new `CCNxControl` instance containing a "List Connections" request.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @return A new `CCNxControl` instance containing the request.
 *
 * Example:
 * @code
 * {
 *     CCNxControl *control = ccnxControl_CreateRouteListRequest();
 *
 *     ...
 *
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateConnectionListRequest(void);

/**
 * Create a new `CCNxControl` instance containing a "List Interfaces" request.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @return A new `CCNxControl` instance containing the request.
 *
 * Example:
 * @code
 * {
 *     CCNxControl *control = ccnxControl_CreateInterfaceListRequest();
 *
 *     ...
 *
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateInterfaceListRequest(void);

/**
 * Create a new `CCNxControl` instance containing a "Pause Input" request.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @return A new `CCNxControl` instance containing the request.
 *
 * Example:
 * @code
 * {
 *   CCNxControl *control = ccnxControl_CreatePauseInputRequest();
 *
 *   ...
 *
 *   ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 * @see {@link cpi_CreatePauseInputRequest}
 */
CCNxControl *ccnxControl_CreatePauseInputRequest(void);


/**
 * Creates a request to flush the output.  The ForwarderConnector will ACK the request.
 *
 * When the user recieves an ACK with the corresponding sequence number as this request, the
 * user knows that all ouptut prior to that request has been processed.
 *
 * @retval non-null An allocated CCnxControl message
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxControl *ccnxControl_CreateFlushRequest(void);

/**
 * Create a new `CCNxControl` instance containing a "Cancel Flow" request.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 * @param [in] name A pointer to a `CCNxName`.
 *
 * @return A new `CCNxControl` instance containing the request.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");
 *     CCNxControl *control = ccnxControl_CreateCancelFlowRequest(name);
 *
 *     ...
 *
 *     ccnxControl_Release(&control);
 *     ccnxName_Release(&name);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateCancelFlowRequest(const CCNxName *name);

/**
 * Create a new `CCNxControl` instance containing a "Create IP Tunnel" request.
 *
 * The new `CCNxControl` instance must eventually be released by calling {@link ccnxControl_Release}.
 *
 * @param [in] tunnel An instance of `CPIInterfaceIPTunnel` to be included.
 * @return A new `CCNxControl` instance containing the request.
 *
 * Example:
 * @code
 * {
 *     struct sockaddr_in sockaddr_any;
 *     memset(&sockaddr_any, 0, sizeof(sockaddr_any));
 *     sockaddr_any.sin_family = PF_INET;
 *     sockaddr_any.sin_addr.s_addr = INADDR_ANY;
 *
 *     CPIAddress *source = cpiAddress_CreateFromInet(&sockaddr_any);
 *
 *     struct sockaddr_in sockaddr_dst;
 *     memset(&sockaddr_dst, 0, sizeof(sockaddr_dst));
 *     sockaddr_dst.sin_family = PF_INET;
 *     sockaddr_dst.sin_port = htons(9999);
 *     inet_pton(AF_INET, "127.0.0.1", &(sockaddr_dst.sin_addr));
 *
 *     CPIAddress *destination = cpiAddress_CreateFromInet(&sockaddr_dst);
 *
 *     CPIInterfaceIPTunnel *tunnel = cpiInterfaceIPTunnel_Create(0, source, destination, IPTUN_TCP);
 *     CCNxControl *control = ccnxControl_CreateIPTunnelRequest(tunnel);
 *
 *     ...
 *
 *     ccnxControl_Release(&control);
 *     cpiInterfaceIPTunnel_Destroy(&tunnel);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_Release}
 */
CCNxControl *ccnxControl_CreateIPTunnelRequest(const CPIInterfaceIPTunnel *tunnel);


CCNxControl *ccnxControl_CreateCacheStoreRequest(bool activate);
CCNxControl *ccnxControl_CreateCacheServeRequest(bool activate);
CCNxControl *ccnxControl_CreateCacheClearRequest();

/**
 * Return true if the specified `CCNxControl` instance is an a CPI request.
 *
 * @param [in] controlMsg A pointer to a `CCNxControl` instance.
 *
 * @return `true` if the specified `CCNxControl` instance is a CPI request.
 * @return `false` if the specified `CCNxControl` instance is not a CPI request.
 *
 * Example:
 * @code
 * {
 *     bool isCPI = ccnxControl_IsCPI(control);
 * }
 * @endcode
 *
 * @see {@link ccnxControl_IsNotification}
 */
bool ccnxControl_IsCPI(const CCNxControl *controlMsg);

/**
 * Return the underlying CPI request from the specified `CCNxControl`.
 *
 * @return A pointer to the underlying CPI request object.
 *
 * Example:
 * @code
 * {
 *     CCNxControl *control = ccnxControl_CreateRouteListRequest();
 *     PARCJSON *json = ccnxControl_GetJson(control);
 *
 *     ...
 *
 *     ccnxControl_Release(&control);
 * }
 * @endcode
 *
 * @see ccnxControl_Release
 */
PARCJSON *ccnxControl_GetJson(const CCNxControl *control);
#endif // libccnx_ccnx_Control_h
