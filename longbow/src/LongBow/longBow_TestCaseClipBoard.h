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
 * @file longBow_TestCaseClipboard.h
 * @ingroup testing
 * @brief LongBow Clipboard shared between the setup, test case, and teardown.
 *
 */
#ifndef LongBow_longBowTestCaseClipBoard_h
#define LongBow_longBowTestCaseClipBoard_h

struct longbow_testcase_clipboard;

/**
 * @typedef LongBowTestCaseClipBoard
 */
typedef struct longbow_testcase_clipboard LongBowTestCaseClipBoard;

/**
 * Create a `LongBowTestCaseClipBoard` containing the shared data pointer.
 *
 * @param [in] shared A pointer to a value that is shared between the setup, test-case, and tear-down functions.
 * @return A pointer to a valid LongBowTestCaseClipBoard instance.
 *
 * @see longBowTestCaseClipBoard_Destroy
 */
LongBowTestCaseClipBoard *longBowTestCaseClipBoard_Create(void *shared);

/**
 *
 * @param [in,out] clipBoardPtr A pointer to a pointer to a LongBowTestCaseClipBoard instance.
 */
void longBowTestCaseClipBoard_Destroy(LongBowTestCaseClipBoard **clipBoardPtr);

/**
 *
 * @param [in] clipBoard A pointer to a valid LongBowTestCaseClipBoard instance.
 * @param [in] shared A pointer to a value that is shared between the setup, test-case, and tear-down functions.
 * @return The previous value of the "clipboard", or NULL if there was no previous value.
 */
LongBowTestCaseClipBoard *longBowTestCaseClipBoard_Set(LongBowTestCaseClipBoard *clipBoard, void *shared);

/**
 *
 * @param [in] clipBoard A pointer to a valid LongBowTestCaseClipBoard instance.
 * @return The value currently stored on the clipboard.
 */
void *longBowTestCaseClipBoard_Get(const LongBowTestCaseClipBoard *clipBoard);
#endif
