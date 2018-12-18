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
 * @file longBow_SubProcess.h
 * @ingroup testing
 * @brief Facilities for running and managing subprocesses.
 *
 */
#ifndef LongBow_longBow_SubProcess_h
#define LongBow_longBow_SubProcess_h

#include <stdbool.h>

struct longbow_subprocess;
typedef struct longbow_subprocess LongBowSubProcess;

/**
 * Start a subprocess.
 *
 * @param [in] path The pathname, either absolute or relative to the current directory, of the program to execute.
 * @param [in] ... A NULL terminated parameter list consisting of the parameters of the executable starting at `argv[0]`.
 *
 * @return A pointer to a `LongBowSubProcess` instance.
 *
 * Example:
 * @code
 * {
 *     LongBowSubProcess *process = longBowSubProcess_Exec("/bin/pwd", "pwd", NULL);
 * }
 * @endcode
 *
 * @see longBowSubProcess_Wait
 * @see longBowSubProcess_Signal
 * @see longBowSubProcess_Terminate
 * @see longBowSubProcess_Wait
 */
LongBowSubProcess *longBowSubProcess_Exec(const char *path, ... /*, (char *)0 */);

/**
 * Destroy a `LongBowSubProcess`
 *
 * If the process is still running it is sent the SIGKILL signal.
 *
 * @param [in] subProcessPtr A pointer to a `LongBowSubProcess` instance.
 *
 * Example:
 * @code
 * {
 *     LongBowSubProcess *process = longBowSubProcess_Exec("/bin/pwd", "pwd", NULL);
 *
 *     longBowSubProcess_Destroy(&process);
 * }
 * @endcode
 *
 * @see longBowSubProcess_Exec
 */
void longBowSubProcess_Destroy(LongBowSubProcess **subProcessPtr);

/**
 * Send a signal to a LongBowSubProcess
 *
 * @param [in] subProcess The LongBowSubProcess to receive the signal.
 * @param [in] signalNumber The signal to send.
 *
 * @return true The signal was successfully sent.
 * @return false The signal was not successfully sent.
 *
 * Example:
 * @code
 * {
 *     longBowSubProcess_Signal(subProcess, SIGTERM);
 * }
 * @endcode
 *
 * @see longBowSubProcess_Terminate
 */
bool longBowSubProcess_Signal(LongBowSubProcess *subProcess, int signalNumber);

/**
 * Send a SIGTERM to a LongBowSubProcess
 *
 * @param [in] subProcess The LongBowSubProcess to receive the signal.
 *
 * @return true The signal was successfully sent.
 * @return false The signal was not successfully sent.
 *
 * Example:
 * @code
 * {
 *     longBowSubProcess_Signal(subProcess, SIGTERM);
 * }
 * @endcode
 *
 * @see longBowSubProcess_Terminate
 */
bool longBowSubProcess_Terminate(LongBowSubProcess *subProcess);

/**
 * Wait for a `LongBowSubProcess` to stop or terminate
 *
 * @param [in] subProcess The LongBowSubProcess to wait for.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
void longBowSubProcess_Wait(LongBowSubProcess *subProcess);

/**
 * Print a human readable representation of the given `LongBowSubProcess`.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] subprocess A pointer to the instance to display.
 *
 * Example:
 * @code
 * {
 *     LongBowSubProcess *process = longBowSubProcess_Exec("/bin/pwd", "pwd", NULL);
 *
 *     longBowSubProcess_Display(process, 0);
 *
 *     longBowSubProcess_Destroy(&process);
 * }
 * @endcode
 *
 */
void longBowSubProcess_Display(const LongBowSubProcess *subprocess, int indentation);
#endif
