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
 * @file cpi_NameRouteType.h
 * @brief Specifies how a route should be matched
 *
 * NOTE: Only LONGEST_MATCH is currently implemented.
 *
 * A LONGEST_MATCH route is a normal CCNx route entry.  It will match any Interest name that is equal to the route prefix
 * or any Interest name that is equal to the router prefix and has additional name components.  Each name component must be
 * exactly equal on a component-by-component basis.
 *
 * An EXACT_MATCH route will not match any longer names.  An Interest name must exactly match the route prefix.
 *
 * A Default route will be used if there are no other matches.
 *
 */

#ifndef libccnx_cpi_NameRouteType_h
#define libccnx_cpi_NameRouteType_h

/**
 * @typedef CPINameRouteType
 * @abstract Enumerates the types of route entries
 * @constant cpiNameRouteType_EXACT_MATCH Specifies an exact match route
 * @constant cpiNameRouteType_LONGEST_MATCH Specifies a longest matching prefix entry (a normal CCNx route)
 * @constant cpiNameRouteType_DEFAULT Specifies a default route that is used if no other entries match
 */
typedef enum {
    cpiNameRouteType_EXACT_MATCH = 1,
    cpiNameRouteType_LONGEST_MATCH = 2,
    cpiNameRouteType_DEFAULT = 3
} CPINameRouteType;

/**
 * Return the string representation of the specified `CPINameRouteType`.
 *
 * The returned string does not need to be freed.
 *
 * @param [in] type The type to represent as a string.
 *
 * @return The string representation of the specified 'CPINameRouteType'.
 *
 * Example:
 * @code
 * {
 *     CPINameRouteType type = cpiNameRouteType_LONGEST_MATCH;
 *
 *     char *name = cpiNameRouteType_ToString(type);
 *
 *     printf("NameRouteType is %s\n", name);
 * }
 * @endcode
 *
 * @see cpiNameRouteType_FromString
 */
const char *cpiNameRouteType_ToString(CPINameRouteType type);

/**
 * Given a string describing a `CPINameRouteType`, return the matching `CPINameRouteType`.
 *
 * If an invalid string is specified, the program will terminate with an IllegalValue exception.
 * Possible values are: "EXACT", "LONGEST", and "DEFAULT".
 *
 * @param [in] str A pointer to a string representation of the desired `CPINameRouteType`.
 *
 * @return The `NameRouteType` matching the specified string.
 *
 * Example:
 * @code
 * {
 *     CPINameRouteType type = cpiNameRouteType_FromString("EXACT");
 * }
 * @endcode
 *
 * @see cpiNameRouteType_ToString
 */
CPINameRouteType cpiNameRouteType_FromString(const char *str);
#endif // libccnx_cpi_NameRouteType_h
