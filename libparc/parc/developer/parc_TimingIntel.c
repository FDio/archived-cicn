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
 * Executes either the RDTSC or RDTSCP instruction, depending on platform availability
 *
 */

#include <config.h>
#include <stdio.h>

#define PARCTIMING_ENABLE
#include <parc/developer/parc_Timing.h>

#ifdef PARCTIMING_INTEL
static bool _useRdtscp = false;
static bool _needCheckRdtscp = true;

#include <cpuid.h>

static void
_checkRdtscp(void)
{
    // See the CPUID instruction for description of the codes.

    // determine the maximum extended information set
    unsigned maxextended = __get_cpuid_max(0x80000000, NULL);

    // RDTSCP status flag is in the 0x800000001 feature set
    const unsigned feature = 0x80000001;
    const unsigned rdtscp_feature = 1 << 27;

    if (maxextended >= feature) {
        unsigned eax, ebx, ecx, edx;

        int success = __get_cpuid(feature, &eax, &ebx, &ecx, &edx);
        if (success) {
            _useRdtscp = (edx & rdtscp_feature ? true : false);
        }
    }
}
#endif // PARCTIMING_INTEL

void
parcTiminIntel_RuntimeInit(void)
{
#ifdef PARCTIMING_INTEL
    if (_needCheckRdtscp) {
        _needCheckRdtscp = false;
        _checkRdtscp();
    }
#endif // PARCTIMING_INTEL
}

void
parcTimingIntel_rdtsc(unsigned *hi, unsigned *lo)
{
    /*
     * Older CPUs do not support RDTSCP, which is the better instruction to use.
     * If we did not detect this opcode in autoconf, use the older RDTSC
     */

#ifdef PARCTIMING_INTEL
    if (_useRdtscp) {
        __asm volatile ("RDTSCP\n\t"
                        "mov %%edx,%0\n\t"
                        "mov %%eax,%1\n\t"
                        "CPUID\n\t" : "=r" (*hi), "=r" (*lo):: "%rax", "%rbx", "%rcx", "%rdx");
    } else {
        __asm volatile ("RDTSC\n\t"
                        "mov %%edx,%0\n\t"
                        "mov %%eax,%1\n\t"
                        "CPUID\n\t" : "=r" (*hi), "=r" (*lo):: "%rax", "%rbx", "%rcx", "%rdx");
    }
#endif  // PARCTIMING_INTEL
}

