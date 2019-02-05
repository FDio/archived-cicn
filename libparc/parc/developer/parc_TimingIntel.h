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
 * @file parc_TimingIntel.h
 * @ingroup developer
 * @brief Macros for timing code
 *
 *  This code uses the Intel recommended benchmarking techniques described
 *  in the whitepaper "How to Benchmakr Code Execution Times on Intel (R) IA-32 and
 *  IA-64 Instruction Set Architectures" available at:
 *
 *  http://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf
 *
 *
 */
#ifndef libparc_parc_TimingIntel_h
#define libparc_parc_TimingIntel_h

#ifdef PARCTIMING_INTEL
#include <stdint.h>
#include <stdbool.h>

/**
 * Reads the TSC via the best available CPU instruction
 *
 * Will execute a RDTSC or RDTSCP instruction followed by an instruction pipeline block
 * CPUID instruction.
 *
 * @param [out] hi The high-order 32-bits
 * @param [out] lo The low-order 32-bits
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcTimingIntel_rdtsc(unsigned *hi, unsigned *lo);

/**
 * Checks initialization for RDTSCP instruction availability
 *
 * Checks if RDTSCP is availalbe on the system and sets a global.
 *
 * Example:
 * @code
 * {
 *     parcTiminIntel_RuntimeInit();
 * }
 * @endcode
 */
void parcTiminIntel_RuntimeInit(void);

#define _private_parcTiming_Init(prefix) \
    parcTiminIntel_RuntimeInit(); \
    static unsigned prefix ## _cycles_low0, prefix ## _cycles_high0; \
    static unsigned prefix ## _cycles_low1, prefix ## _cycles_high1; \
   \
    __asm volatile ("CPUID\n\t" "RDTSC\n\t" \
                    "mov %%edx, %0\n\t" \
                    "mov %%eax, %1\n\t" : "=r" (prefix ## _cycles_high0), "=r" (prefix ## _cycles_low0):: \
                    "%rax", "%rbx", "%rcx", "%rdx"); \
    parcTimingIntel_rdtsc(&(prefix ## _cycles_high1), &(prefix ## _cycles_low1)); \
    __asm volatile ("CPUID\n\t" \
                    "RDTSC\n\t" \
                    "mov %%edx, %0\n\t" \
                    "mov %%eax, %1\n\t" : "=r" (prefix ## _cycles_high0), "=r" (prefix ## _cycles_low0):: \
                    "%rax", "%rbx", "%rcx", "%rdx"); \
    parcTimingIntel_rdtsc(&(prefix ## _cycles_high1), &(prefix ## _cycles_low1));

#define _private_parcTiming_Start(prefix) \
    parcTimingIntel_rdtsc(&(prefix ## _cycles_high0), &(prefix ## _cycles_low0));

#define _private_parcTiming_Stop(prefix) \
    parcTimingIntel_rdtsc(&(prefix ## _cycles_high1), &(prefix ## _cycles_low1));

#define _private_parcTiming_CalculateStartTime(prefix) (((uint64_t) prefix ## _cycles_high0 << 32) | prefix ## _cycles_low0)
#define _private_parcTiming_CalculateStopTime(prefix) (((uint64_t) prefix ## _cycles_high1 << 32) | prefix ## _cycles_low1)

#define _private_parcTiming_Delta(prefix) \
    (_private_parcTiming_CalculateStopTime(prefix) - _private_parcTiming_CalculateStartTime(prefix))

// No teardown work that we need to do
#define _private_parcTiming_Fini(prefix)

#endif // PARCTIMING_INTEL
#endif // libparc_parc_TimingIntel_h

