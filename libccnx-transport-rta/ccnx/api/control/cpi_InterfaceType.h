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
 * @file cpi_InterfaceTypes.h
 * @brief CPI INterface Types
 *
 *
 */
#ifndef libccnx_cpi_InterfaceTypes_h
#define libccnx_cpi_InterfaceTypes_h

typedef enum {
    CPI_IFACE_LOOPBACK = 1,
    CPI_IFACE_ETHERNET = 2,
    CPI_IFACE_LOCALAPP = 3,
    CPI_IFACE_TUNNEL = 4,
    CPI_IFACE_GROUP = 5
} CPIInterfaceType;

typedef enum {
    CPI_IFACE_UNKNOWN = 0,
    CPI_IFACE_UP = 1,
    CPI_IFACE_DOWN = 2
} CPIInterfaceStateType;

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param [in] type An instance of `CPIInterfaceType`.
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiInterfaceType_ToString(CPIInterfaceType type);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param str A nul-terminated C string.
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIInterfaceType cpiInterfaceType_FromString(const char *str);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param type A CPIInterfaceStateType value.
 * @return A nul-terminated C string.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiInterfaceStateType_ToString(CPIInterfaceStateType type);

/**
 * <#OneLineDescription#>
 *
 *   <#Discussion#>
 *
 * @param str A nul-terminated C string.
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CPIInterfaceStateType cpiInterfaceStateType_FromString(const char *str);
#endif // libccnx_cpi_InterfaceTypes_h
