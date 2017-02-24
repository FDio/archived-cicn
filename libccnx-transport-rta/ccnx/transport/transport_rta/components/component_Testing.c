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
 * the Testing component does not implement any of its methods.  This means it may be inserted above and
 * below another component so a unit test can look at its queues
 */

#include <config.h>
#include <stdio.h>

#include <LongBow/runtime.h>

#include <ccnx/transport/transport_rta/rta_Transport.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/core/rta_Connection.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>

#include <ccnx/transport/transport_rta/components/component_Testing.h>

RtaComponentOperations testing_null_ops = {
    .init          = NULL, /* init */
    .open          = NULL, /* open */
    .upcallRead    = NULL, /* upcall read */
    .upcallEvent   = NULL,
    .downcallRead  = NULL, /* downcall read */
    .downcallEvent = NULL,
    .close         = NULL, /* closer */
    .release       = NULL  /* release */
};

// ==================
// NULL
