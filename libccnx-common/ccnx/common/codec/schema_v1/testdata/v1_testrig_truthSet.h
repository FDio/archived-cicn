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
 * Contains tables of all the packets.  May be used for automated testing.  Also used by write_packets utility.
 *
 */

#include <ccnx/common/codec/testdata/testdata_common.h>

#include "v1_interest_nameA.h"
#include "v1_interest_nameA_badcrc32c.h"
#include "v1_interest_nameA_crc32c.h"
#include "v1_interest_bad_validation_alg.h"
#include "v1_interest_validation_alg_overrun.h"

#include "v1_content_nameA_crc32c.h"
#include "v1_content_nameA_keyid1_rsasha256.h"
#include "v1_content_zero_payload.h"
#include "v1_content_no_payload.h"

#include "v1_cpi_add_route.h"
#include "v1_cpi_add_route_crc32c.h"

// terminated with NULL packet entry
__attribute__((unused))
static TruthTable v1_interests_truthSet [] = {
    // tests in alphabetical order
    v1_interest_nameA_truthTable,
    v1_interest_nameA_badcrc32c_truthTable,
    v1_interest_nameA_crc32c_truthTable,
    v1_interest_bad_validation_alg_truthTable,
    v1_interest_validation_alg_overrun_truthTable,
    // the end of table marker
    { .packet = NULL,                             .expectedError= 0, .entry = NULL }
};

// terminated with NULL packet entry
__attribute__((unused))
static TruthTable v1_contentObject_truthSet [] = {
    v1_content_nameA_crc32c_truthTable,
    v1_content_nameA_keyid1_rsasha256_truthTable,
    v1_content_zero_payload_truthTable,
    v1_content_no_payload_truthTable,

    // the end of table marker
    { .packet = NULL,                            .expectedError= 0, .entry = NULL }
};

__attribute__((unused))
static TruthTable v1_cpi_truthSet [] = {
    v1_cpi_add_route_truthTable,
    v1_cpi_add_route_crc32c_truthTable,
    // the end of table marker
    { .packet = NULL,                  .expectedError= 0, .entry = NULL }
};
