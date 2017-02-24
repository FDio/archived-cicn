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

#include <LongBow/runtime.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/queue.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_EventQueue.h>

#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_Framework_Services.h>

#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>
#include <ccnx/transport/transport_rta/core/rta_ConnectionTable.h>
#include <ccnx/transport/transport_rta/core/rta_ComponentStats.h>
#include <ccnx/transport/common/transport_Message.h>
#include <ccnx/transport/common/transport_private.h>

#include <ccnx/transport/transport_rta/connectors/connector_Api.h>
#include <ccnx/transport/transport_rta/connectors/connector_Forwarder.h>
#include <ccnx/transport/transport_rta/components/component_Codec.h>
#include <ccnx/transport/transport_rta/components/component_Flowcontrol.h>
#include <ccnx/transport/transport_rta/components/component_Testing.h>

#include <ccnx/transport/transport_rta/config/config_ProtocolStack.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#define MAX_STACK_DEPTH 10

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

const char *RtaComponentNames[LAST_COMPONENT] =
{
    "API",              // 0
    "FC_NONE",
    "FC_VEGAS",
    "FC_PIPELINE",
    "VERIFY_NONE",      // 4
    "VERIFY_ENUMERATED",
    "VERIFY_LOCATOR",
    "CODEC_NONE",
    NULL,               // 8
    "CODEC_TLV",
    "CODEC_CCNB",
    "CODE_FLAN",
    NULL,               // 12
    "FWD_LOCAL",
    "FWD_FLAN",
    "FWD_CCND",         // 15
    "TESTING_UPPER",
    "TESTING_LOWER",    // 17
    "CCND_REGISTRAR",
    "FWD_METIS"
};

struct protocol_stack {
    int stack_id;

    // used during configuration to indicate if configured
    int config_codec;

    RtaFramework *framework;

    // They key value pairs passed to open.  The api must
    // keep this memory valid for as long as the connection is open
    PARCJSON *params;

    // the inter-component queues
    unsigned component_count;
    PARCEventQueuePair *queue_pairs[MAX_STACK_DEPTH];
    RtaComponents components[MAX_STACK_DEPTH];

    // queues assigned to components
    struct component_queues {
        PARCEventQueue *up;
        PARCEventQueue *down;
    } *component_queues[LAST_COMPONENT];
    RtaComponentOperations component_ops[LAST_COMPONENT];
    void *component_state[LAST_COMPONENT];

    // stack-wide stats
    RtaComponentStats *stack_stats[LAST_COMPONENT];


    // state change events are disabled during initial setup and teardown
    bool stateChangeEventsEnabled;
};

static void set_queue_pairs(RtaProtocolStack *stack, RtaComponents comp_type);
static int configure_ApiConnector(RtaProtocolStack *stack, RtaComponents comp_type, RtaComponentOperations ops);
static int configure_Component(RtaProtocolStack *stack, RtaComponents comp_type, RtaComponentOperations ops);
static int configure_FwdConnector(RtaProtocolStack *stack, RtaComponents comp_type, RtaComponentOperations ops);

// ========================================

RtaFramework *
rtaProtocolStack_GetFramework(RtaProtocolStack *stack)
{
    assertNotNull(stack, "called with null stack");
    return stack->framework;
}

RtaProtocolStack *
rtaProtocolStack_Create(RtaFramework *framework, PARCJSON *params, int stack_id)
{
    RtaProtocolStack *stack = parcMemory_AllocateAndClear(sizeof(RtaProtocolStack));
    assertNotNull(stack, "%9" PRIu64 " parcMemory_AllocateAndClear returned NULL\n",
                  rtaFramework_GetTicks(stack->framework));

    stack->stateChangeEventsEnabled = false;

    stack->params = parcJSON_Copy(params);

    assertNotNull(stack->params, "SYSTEM key is NULL in params");

    assertNotNull(framework, "Parameter framework may not be null");

    stack->framework = framework;
    stack->stack_id = stack_id;

    // create all the buffer pairs
    for (int i = 0; i < MAX_STACK_DEPTH; i++) {
        stack->queue_pairs[i] = parcEventQueue_CreateConnectedPair(rtaFramework_GetEventScheduler(stack->framework));

        assertNotNull(stack->queue_pairs[i], "parcEventQueue_CreateConnectedPair returned NULL index %d", i);
        if (stack->queue_pairs[i] == NULL) {
            for (int j = 0; j < i; j++) {
                parcEventQueue_DestroyConnectedPair(&(stack->queue_pairs[j]));
            }

            parcMemory_Deallocate((void **) &stack);
            return NULL;
        }

        // set them all to normal priority.  The command port is high priority. External buffes are low priority.
        parcEventQueue_SetPriority(parcEventQueue_GetConnectedUpQueue(stack->queue_pairs[i]), PARCEventPriority_Normal);
        parcEventQueue_SetPriority(parcEventQueue_GetConnectedDownQueue(stack->queue_pairs[i]), PARCEventPriority_Normal);

        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s create buffer pair %p <-> %p\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(stack)),
                   __func__,
                   (void *) parcEventQueue_GetConnectedUpQueue(stack->queue_pairs[i]),
                   (void *) parcEventQueue_GetConnectedDownQueue(stack->queue_pairs[i]));
        }
    }

    for (int i = 0; i < LAST_COMPONENT; i++) {
        stack->stack_stats[i] = rtaComponentStats_Create(NULL, i);
    }

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s created stack %d at %p\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(stack)),
               __func__,
               stack_id,
               (void *) stack);
    }

    stack->stateChangeEventsEnabled = true;

    return stack;
}

/**
 * Opens a connection inside the protocol stack: it calls open() on each component.
 *
 * Returns 0 on success, -1 on error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaProtocolStack_Open(RtaProtocolStack *stack, RtaConnection *connection)
{
    assertNotNull(stack, "called with null stack\n");

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s stack_id %d opening conn %p api_fd %d\n",
               rtaFramework_GetTicks(rtaProtocolStack_GetFramework(stack)),
               __func__,
               stack->stack_id,
               (void *) connection, rtaConnection_GetApiFd(connection));
    }

    // call all the opens, except the api

    // need to disable events during creation to avoid calling the event notifier
    // of a component before the component sees the "open" call for this connection
    stack->stateChangeEventsEnabled = false;
    for (int i = 0; i < stack->component_count; i++) {
        RtaComponents comp = stack->components[i];
        if (stack->component_ops[comp].open != NULL &&
            stack->component_ops[comp].open(connection) != 0) {
            fprintf(stderr, "%s component %d failed open\n", __func__, i);
            abort();
            return -1;
        }
    }
    stack->stateChangeEventsEnabled = true;

    return 0;
}

/*
 * Closes a connection but does not touch stack->connection_head
 */
static int
internal_Stack_Close(RtaProtocolStack *stack, RtaConnection *conn)
{
    int i;

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s stack_id %d closing stack %p conn %p\n",
               rtaFramework_GetTicks(rtaConnection_GetFramework(conn)),
               __func__,
               stack->stack_id,
               (void *) stack,
               (void *) conn);
    }

    rtaConnection_SetState(conn, CONN_CLOSED);

    // call all the opens
    for (i = 0; i < stack->component_count; i++) {
        RtaComponents comp = stack->components[i];
        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s calling close for %s\n",
                   rtaFramework_GetTicks(rtaConnection_GetFramework(conn)), __func__, RtaComponentNames[comp]);
        }

        if (stack->component_ops[comp].close != NULL &&
            stack->component_ops[comp].close(conn) != 0) {
            fprintf(stderr, "%s component %d failed open\n", __func__, i);
            abort();
            return -1;
        }
    }

    return 0;
}

/**
 * Calls the close() function of each component in the protocol stack.
 *
 * This is typically called from inside the API connector when it processes
 * a CLOSE json message.
 * Returns 0 success, -1 error.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaProtocolStack_Close(RtaProtocolStack *stack, RtaConnection *conn)
{
    assertNotNull(stack, "called with null stack\n");
    assertNotNull(conn, "called with null connection\n");

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s stack_id %d stack %p conn %p\n",
               rtaFramework_GetTicks(rtaConnection_GetFramework(conn)),
               __func__,
               stack->stack_id,
               (void *) stack,
               (void *) conn);
    }


    internal_Stack_Close(stack, conn);

    return 0;
}

/**
 * Calls the release() function of all components.
 * Drains all the component queues.
 *
 * This is called from rtaFramework_DestroyStack, who is responsible for closing
 * all the connections in it before calling this.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
rtaProtocolStack_Destroy(RtaProtocolStack **stackPtr)
{
    RtaProtocolStack *stack;

    assertNotNull(stackPtr, "%s called with null pointer to stack\n", __func__);

    stack = *stackPtr;
    assertNotNull(stack, "%s called with null stack dereference\n", __func__);

    if (DEBUG_OUTPUT) {
        printf("%s stack_id %d destroying stack %p\n",
               __func__,
               stack->stack_id,
               (void *) stack);
    }

    stack->stateChangeEventsEnabled = false;

    // call all the release functions
    for (int i = 0; i < stack->component_count; i++) {
        RtaComponents comp = stack->components[i];
        if (stack->component_ops[comp].release != NULL &&
            stack->component_ops[comp].release(stack) != 0) {
            fprintf(stderr, "%s component %d failed release\n", __func__, i);
            abort();
        }
    }

    for (int i = 0; i < MAX_STACK_DEPTH; i++) {
        TransportMessage *tm;
        while ((tm = rtaComponent_GetMessage(parcEventQueue_GetConnectedUpQueue(stack->queue_pairs[i]))) != NULL) {
            assertFalse(1, "%s we should never execute the body, it should just drain\n", __func__);
        }

        while ((tm = rtaComponent_GetMessage(parcEventQueue_GetConnectedDownQueue(stack->queue_pairs[i]))) != NULL) {
            assertFalse(1, "%s we should never execute the body, it should just drain\n", __func__);
        }

        if (DEBUG_OUTPUT) {
            printf("%9" PRIu64 " %s destroy buffer pair %p <-> %p\n",
                   rtaFramework_GetTicks(rtaProtocolStack_GetFramework(stack)),
                   __func__,
                   (void *) parcEventQueue_GetConnectedUpQueue(stack->queue_pairs[i]),
                   (void *) parcEventQueue_GetConnectedDownQueue(stack->queue_pairs[i]));
        }

        parcEventQueue_DestroyConnectedPair(&(stack->queue_pairs[i]));
    }

    for (int i = 0; i < LAST_COMPONENT; i++) {
        if (stack->component_queues[i]) {
            parcMemory_Deallocate((void **) &(stack->component_queues[i]));
        }
    }

    for (int i = 0; i < LAST_COMPONENT; i++) {
        rtaComponentStats_Destroy(&stack->stack_stats[i]);
    }


    parcJSON_Release(&stack->params);
    memset(stack, 0, sizeof(RtaProtocolStack));

    parcMemory_Deallocate((void **) &stack);
    *stackPtr = NULL;
}


PARCEventQueue *
rtaProtocolStack_GetPutQueue(RtaProtocolStack *stack, RtaComponents component, RtaDirection direction)
{
    assertNotNull(stack, "%s called with null stack\n", __func__);

    if (direction == RTA_UP) {
        return stack->component_queues[component]->up;
    } else {
        return stack->component_queues[component]->down;
    }
}


/**
 * Look up the symbolic name of the queue.  Do not free the return.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *
rtaProtocolStack_GetQueueName(RtaProtocolStack *stack, PARCEventQueue *queue)
{
    int component;
    for (component = 0; component <= LAST_COMPONENT; component++) {
        if (stack->component_queues[component]) {
            if (stack->component_queues[component]->up == queue) {
                return RtaComponentNames[component];
            }
            if (stack->component_queues[component]->down == queue) {
                return RtaComponentNames[component];
            }
        }
    }
    trapUnexpectedState("Could not find queue %p in stack %p", (void *) queue, (void *) stack);
}

// =================================================
// =================================================

static RtaComponents
getComponentTypeFromName(const char *name)
{
    int i;

    if (name == NULL) {
        return UNKNOWN_COMPONENT;
    }

    for (i = 0; i < LAST_COMPONENT; i++) {
        if (RtaComponentNames[i] != NULL) {
            if (strncasecmp(RtaComponentNames[i], name, 16) == 0) {
                return (RtaComponents) i;
            }
        }
    }
    return UNKNOWN_COMPONENT;
}


/**
 * Calls the confguration routine for each component in the stack
 *
 * Builds an array list of everything in the JSON configuration, then
 * calls its configuation routine.
 *
 * The connecting event queues are disabled at this point.
 *
 * @param [in,out] stack The Protocol Stack to operate on
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
rtaProtocolStack_ConfigureComponents(RtaProtocolStack *stack)
{
    PARCArrayList *componentNameList;
    componentNameList = protocolStack_GetComponentNameArray(stack->params);
    assertTrue(parcArrayList_Size(componentNameList) < MAX_STACK_DEPTH,
               "Too many components in a stack size %zu\n",
               parcArrayList_Size(componentNameList));


    for (int i = 0; i < parcArrayList_Size(componentNameList); i++) {
        // match it to a component type
        const char *comp_name = parcArrayList_Get(componentNameList, i);
        RtaComponents comp_type = getComponentTypeFromName(comp_name);

        // this could be sped up slightly by putting the ops structures
        // in an array
        switch (comp_type) {
            case API_CONNECTOR:
                configure_ApiConnector(stack, comp_type, api_ops);
                break;

            case FC_NONE:
                trapIllegalValue(comp_type, "Null flowcontroller no longer supported");
                break;
            case FC_VEGAS:
                configure_Component(stack, comp_type, flow_vegas_ops);
                break;
            case FC_PIPELINE:
                abort();
                break;

            case CODEC_NONE:
                trapIllegalValue(comp_type, "Null codec no longer supported");
                break;
            case CODEC_TLV:
                configure_Component(stack, comp_type, codec_tlv_ops);
                break;

            case FWD_NONE:
                abort();
                break;
            case FWD_LOCAL:
                configure_FwdConnector(stack, comp_type, fwd_local_ops);
                break;

            case FWD_METIS:
                configure_FwdConnector(stack, comp_type, fwd_metis_ops);
                break;

            case TESTING_UPPER:
            // fallthrough
            case TESTING_LOWER:
                configure_Component(stack, comp_type, testing_null_ops);
                break;


            default:
                fprintf(stderr, "%s unsupported component type %s\n", __func__, comp_name);
                abort();
        }
    }
    parcArrayList_Destroy(&componentNameList);
}

static bool
rtaProtocolStack_InitializeComponents(RtaProtocolStack *stack)
{
    // Call all the inits
    for (int i = 0; i < LAST_COMPONENT; i++) {
        int res = 0;
        if (stack->component_ops[i].init != NULL) {
            res = stack->component_ops[i].init(stack);
        }

        if (res != 0) {
            fprintf(stderr, "%s opener for layer %d failed\n", __func__, i);
            trapUnrecoverableState("Error Initializing the components")
            return false;
        }
    }
    return true;
}

/**
 * Enables events on all the queues between components
 *
 * Enables events on each queue.
 *
 * @param [in,out] stack The PRotocol stack
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
rtaProtocolStack_EnableComponentQueues(RtaProtocolStack *stack)
{
    // enable all the events on intermediate queues
    for (int i = 0; i < stack->component_count; i++) {
        RtaComponents component = stack->components[i];
        PARCEventQueue *upQueue = stack->component_queues[component]->up;
        if (upQueue != NULL) {
            parcEventQueue_Enable(upQueue, PARCEventType_Read);
        }

        PARCEventQueue *downQueue = stack->component_queues[component]->down;
        if (downQueue != NULL) {
            parcEventQueue_Enable(downQueue, PARCEventType_Read);
        }
    }
}

/*
 * Called from transportRta_Open()
 *
 * Returns 0 for success, -1 on error (connection not made)
 */
int
rtaProtocolStack_Configure(RtaProtocolStack *stack)
{
    assertNotNull(stack, "%s called with null stack\n", __func__);

    rtaProtocolStack_ConfigureComponents(stack);

    bool initSuccess = rtaProtocolStack_InitializeComponents(stack);
    if (!initSuccess) {
        return -1;
    }

    rtaProtocolStack_EnableComponentQueues(stack);

    return 0;
}

/*
 * Domain is the top-level key, e.g. SYSTEM or USER
 */
PARCJSON *
rtaProtocolStack_GetParam(RtaProtocolStack *stack, const char *domain, const char *key)
{
    assertNotNull(stack, "%s called with null stack\n", __func__);
    assertNotNull(domain, "%s called with null domain\n", __func__);
    assertNotNull(key, "%s called with null key\n", __func__);

    PARCJSONValue *value = parcJSON_GetValueByName(stack->params, domain);
    assertNotNull(value, "Did not find domain %s in protocol stack parameters", domain);
    if (value == NULL) {
        return NULL;
    }
    PARCJSON *domainJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(domainJson, key);
    assertNotNull(value, "Did not find key %s in protocol stack parameters", key);
    return parcJSONValue_GetJSON(value);
}

unsigned
rtaProtocolStack_GetNextConnectionId(RtaProtocolStack *stack)
{
    assertNotNull(stack, "Parameter stack must be a non-null RtaProtocolStack pointer.");
    return rtaFramework_GetNextConnectionId(stack->framework);
}

RtaComponentStats *
rtaProtocolStack_GetStats(const RtaProtocolStack *stack, RtaComponents type)
{
    assertTrue(type < LAST_COMPONENT, "invalid type %d\n", type);
    return stack->stack_stats[type];
}

static void
printSingleTuple(FILE *file, const struct timeval *timeval, const RtaProtocolStack *stack, RtaComponents componentType, RtaComponentStatType stat)
{
    RtaComponentStats *stats = rtaProtocolStack_GetStats(stack, componentType);

    fprintf(file, "{ \"stackId\" : %d, \"component\" : \"%s\", \"name\" : \"%s\", \"value\" : %" PRIu64 ", \"timeval\" : %ld.%06u }\n",
            stack->stack_id,
            RtaComponentNames[componentType],
            rtaComponentStatType_ToString(stat),
            rtaComponentStats_Get(stats, stat),
            timeval->tv_sec,
            (unsigned) timeval->tv_usec
            );
}

PARCArrayList *
rtaProtocolStack_GetStatistics(const RtaProtocolStack *stack, FILE *file)
{
    PARCArrayList *list = parcArrayList_Create(NULL);

    struct timeval timeval;
    gettimeofday(&timeval, NULL);

    // This does not fill in the array list
    for (int componentIndex = 0; componentIndex < stack->component_count; componentIndex++) {
        RtaComponents componentType = stack->components[componentIndex];
        printSingleTuple(file, &timeval, stack, componentType, STATS_OPENS);
        printSingleTuple(file, &timeval, stack, componentType, STATS_CLOSES);
        printSingleTuple(file, &timeval, stack, componentType, STATS_UPCALL_IN);
        printSingleTuple(file, &timeval, stack, componentType, STATS_UPCALL_OUT);
        printSingleTuple(file, &timeval, stack, componentType, STATS_DOWNCALL_IN);
        printSingleTuple(file, &timeval, stack, componentType, STATS_DOWNCALL_OUT);
    }

    return list;
}


// =============================================

static void
set_queue_pairs(RtaProtocolStack *stack, RtaComponents comp_type)
{
    //PARCEventQueuePair *component_queues[LAST_COMPONENT];
    // Save references to the OUTPUT queues used by a specific component.
    if (stack->component_queues[comp_type] == NULL) {
        stack->component_queues[comp_type] = parcMemory_AllocateAndClear(sizeof(struct component_queues));
    }

    stack->component_queues[comp_type]->up =
        parcEventQueue_GetConnectedUpQueue(stack->queue_pairs[stack->component_count - 1]);

    stack->component_queues[comp_type]->down =
        parcEventQueue_GetConnectedDownQueue(stack->queue_pairs[stack->component_count]);

    // Set callbacks on the INPUT queues read by a specific component
    parcEventQueue_SetCallbacks(stack->component_queues[comp_type]->up,
                                stack->component_ops[comp_type].downcallRead,
                                NULL,
                                stack->component_ops[comp_type].downcallEvent,
                                (void *) stack);

    parcEventQueue_SetCallbacks(stack->component_queues[comp_type]->down,
                                stack->component_ops[comp_type].upcallRead,
                                NULL,
                                stack->component_ops[comp_type].upcallEvent,
                                (void *) stack);
}


static int
configure_ApiConnector(RtaProtocolStack *stack, RtaComponents comp_type, RtaComponentOperations ops)
{
    if (stack->component_queues[comp_type] == NULL) {
        stack->component_queues[comp_type] = parcMemory_AllocateAndClear(sizeof(struct component_queues));
    }

    assertNotNull(stack->component_queues[comp_type], "called with null component_queue");
    assertNotNull(stack->queue_pairs[stack->component_count], "called with null queue_pair");

    // This wires the bottom half of the API Connector to the streams.
    // It does not do the top half, which is in the connector's INIT

    stack->components[stack->component_count] = comp_type;
    stack->component_ops[comp_type] = ops;

    stack->component_queues[comp_type]->down =
        parcEventQueue_GetConnectedDownQueue(stack->queue_pairs[stack->component_count]);

    parcEventQueue_SetCallbacks(stack->component_queues[comp_type]->down,
                                stack->component_ops[comp_type].upcallRead,
                                NULL,
                                stack->component_ops[comp_type].upcallEvent,
                                (void *) stack);

    stack->component_count++;
    return 0;
}

static int
configure_Component(RtaProtocolStack *stack, RtaComponents comp_type, RtaComponentOperations ops)
{
    stack->component_ops[comp_type] = ops;
    stack->components[stack->component_count] = comp_type;
    set_queue_pairs(stack, comp_type);
    stack->component_count++;
    return 0;
}


static int
configure_FwdConnector(RtaProtocolStack *stack, RtaComponents comp_type, RtaComponentOperations ops)
{
    stack->component_ops[comp_type] = ops;
    stack->components[stack->component_count] = comp_type;

    // We only set the upcall buffers.  The down buffers
    // are controlled by the forwarder connector
    if (stack->component_queues[comp_type] == NULL) {
        stack->component_queues[comp_type] = parcMemory_AllocateAndClear(sizeof(struct component_queues));
    }

    stack->component_queues[comp_type]->up =
        parcEventQueue_GetConnectedUpQueue(stack->queue_pairs[stack->component_count - 1]);

    parcEventQueue_SetCallbacks(stack->component_queues[comp_type]->up,
                                stack->component_ops[comp_type].downcallRead,
                                NULL,
                                stack->component_ops[comp_type].downcallEvent,
                                (void *) stack);

    stack->component_count++;
    return 0;
}

int
rtaProtocolStack_GetStackId(RtaProtocolStack *stack)
{
    return stack->stack_id;
}

void
rtaProtocolStack_ConnectionStateChange(RtaProtocolStack *stack, void *connection)
{
    if (stack->stateChangeEventsEnabled) {
        for (int componentIndex = 0; componentIndex < stack->component_count; componentIndex++) {
            RtaComponents componentType = stack->components[componentIndex];
            if (stack->component_ops[componentType].stateChange != NULL) {
                stack->component_ops[componentType].stateChange(connection);
            }
        }
    }
}
