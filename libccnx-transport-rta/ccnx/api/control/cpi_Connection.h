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
 * @file cpi_Connection.h
 * @brief Represents a point-to-point tunnel over IP.
 *
 * The carries can be UDP, TCP, or GRE
 *
 * We use InterfaceGeneric to back this type.  We always use 2 addresses in the address list.
 * Address 0 is the source and address 1 is the destination.
 *
 */
#ifndef libccnx_cpi_Connection_h
#define libccnx_cpi_Connection_h

#include <ccnx/api/control/cpi_InterfaceType.h>
#include <ccnx/api/control/cpi_Address.h>

struct cpi_connection;
typedef struct cpi_connection CPIConnection;

typedef enum {
    cpiConnection_GRE,
    cpiConnection_TCP,
    cpiConnection_UDP,
    cpiConnection_MULTICAST,
    cpiConnection_L2
} CPIConnectionType;

/**
 * Return a static, nul-terminated C string representing the given `CPIConnectionType`
 *
 * @param type A valid CPIConnectionType value.
 * @return A static, nul-terminated C string
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiConnectionType_ToString(CPIConnectionType type);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionType cpiConnectionType_FromString(const char *typeAsString);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCBufferComposer *cpiConnectionType_BuildString(CPIConnectionType type, PARCBufferComposer *composer);

/**
 * A representation of a Connection, being two addresses and a type
 *
 *   <#Discussion#>
 *
 * @param ifidx The interface index
 * @param source is the local address
 * @param destination is the remote address
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnection *cpiConnection_Create(unsigned ifidx, CPIAddress *source, CPIAddress *destination, CPIConnectionType connType);


CPIConnection *cpiConnection_Acquire(const CPIConnection *conn);

/**
 * Creates a reference counted copy
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return An allocated copy, you must destroy it
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnection *cpiConnection_Copy(const CPIConnection *conn);

/**
 * Reference counted release
 *
 *   Only on the last reference will the call free the contents.
 *
 * @param <#param1#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void cpiConnection_Release(CPIConnection **connPtr);

/**
 * A connection may be up, down, or don't know state.
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void cpiConnection_SetState(CPIConnection *conn, CPIInterfaceStateType state);

/**
 * Returns the interface index
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return The interface index
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
unsigned cpiConnection_GetIndex(const CPIConnection *conn);

/**
 * The source address
 *
 *   This is not a copy, it is the pointer to what is in the object.  If you
 *   want to save it, make a copy.
 *
 * @param <#param1#>
 * @return Do not destroy it.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const CPIAddress *cpiConnection_GetSourceAddress(const CPIConnection *conn);

/**
 * The destination (remote) address
 *
 *   This is not a copy, it is the pointer to what is in the object.  If you
 *   want to save it, make a copy.
 *
 * @param <#param1#>
 * @return Do not destroy it.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const CPIAddress *cpiConnection_GetDestinationAddress(const CPIConnection *conn);

/**
 * The type of connection
 *
 *   A connection may be a TCP tunnel, UDP tunnel, IP multicast overlay,
 *   PF_LOCAL connection, or a layer 2 connection.
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnectionType         cpiConnection_GetConnectionType(const CPIConnection *conn);

/**
 * The connection state, Up, Down, or Don't Know
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIInterfaceStateType  cpiConnection_GetState(const CPIConnection *conn);

/**
 * Determine if two CPIConnection instances are equal.
 *
 * Two CPIConnection instances are equal if, and only if,
 * (a) the interface index is the same, (b) the connection types are the same,
 * (c) the connection state is the same, (d) the source address are the same, and
 * (e) the destination addresses are the same.
 *
 * The following equivalence relations on non-null `CPIConnection` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `CPIConnection_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `cpiConnection_Equals(x, y)` must return true if and only if
 *        `cpiConnection_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `cpiConnection_Equals(x, y)` returns true and
 *        `cpiConnection_Equals(y, z)` returns true,
 *        then  `cpiConnection_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `cpiConnection_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `cpiConnection_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CPIConnection` instance.
 * @param b A pointer to a `CPIConnection` instance.
 * @return true if the two `CPIConnection` instances are equal.
 *
 * Example:
 * @code
 * {
 *    CPIConnection *a = cpiConnection_Create();
 *    CPIConnection *b = cpiConnection_Create();
 *
 *    if (cpiConnection_Equals(a, b)) {
 *        // true
 *    } else {
 *        // false
 *    }
 * }
 * @endcode
 */
bool cpiConnection_Equals(const CPIConnection *a, const CPIConnection *b);

/**
 * A JSON representation of the connection
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return An allocated object that you must destroy
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpiConnection_ToJson(const CPIConnection *conn);

/**
 * Creates a Connection object based on a JSON representation.
 *
 *   Will assert if there's a parsing error
 *
 * @param <#param1#>
 * @return An allocated connection that you must destroy
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIConnection *cpiConnection_CreateFromJson(PARCJSON *json);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
char *cpiConnection_ToString(const CPIConnection *connection);
#endif // libccnx_cpi_Connection_h
