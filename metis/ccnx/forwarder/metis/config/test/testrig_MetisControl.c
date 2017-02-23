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
 * Common operations for the metisControl tests.  This C module
 * is intended to be #include'd in to each test.
 *
 */

#include <LongBow/unit-test.h>

#include "../metis_ControlState.c"
#include <parc/algol/parc_SafeMemory.h>
#include <ccnx/forwarder/metis/config/metis_CommandParser.h>
#include <ccnx/api/control/controlPlaneInterface.h>

typedef struct test_data {
    MetisControlState *state;
    unsigned writeread_count;

    // If the user specifies this, it will be used as the reply to all test_WriteRead calls
    CCNxControl * (*customWriteReadReply)(void *userdata, CCNxMetaMessage * messageToWrite);
} TestData;

/**
 * As part of the testrig, we simply create a CPIAck of the request message.
 * We also increment the call count in TestData.
 *
 * If the user specified a customWriteReadReply function, we will call that to get
 * the specific response to send.
 */
static CCNxMetaMessage *
test_WriteRead(void *userdata, CCNxMetaMessage *messageToWrite)
{
    TestData *data = (TestData *) userdata;
    data->writeread_count++;

    assertTrue(ccnxMetaMessage_IsControl(messageToWrite), "messageToWrite is not a control message");

    CCNxControl *response;
    CCNxMetaMessage *result;

    if (data->customWriteReadReply == NULL) {
        CCNxControl *request = ccnxMetaMessage_GetControl(messageToWrite);
        PARCJSON *json = ccnxControl_GetJson(request);
        PARCJSON *jsonAck = cpiAcks_CreateAck(json);

        response = ccnxControl_CreateCPIRequest(jsonAck);
        result = ccnxMetaMessage_CreateFromControl(response);

        parcJSON_Release(&jsonAck);
        ccnxControl_Release(&response);
    } else {
        response = data->customWriteReadReply(userdata, messageToWrite);
        assertTrue(ccnxMetaMessage_IsControl(response), "response is not a control message");
        result = ccnxMetaMessage_CreateFromControl(response);
        ccnxControl_Release(&response);
    }

    return result;
}

static void
testrigMetisControl_commonSetup(const LongBowTestCase *testCase)
{
    TestData *data = parcMemory_AllocateAndClear(sizeof(TestData));
    assertNotNull(data, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(TestData));
    memset(data, 0, sizeof(TestData));

    data->state = metisControlState_Create(data, test_WriteRead);
    longBowTestCase_SetClipBoardData(testCase, data);
}

static void
testrigMetisControl_CommonTeardown(const LongBowTestCase *testCase)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    metisControlState_Destroy(&data->state);
    parcMemory_Deallocate((void **) &data);
}

/**
 * Verify that a Command Create operated correctly
 *
 * We verify the basic properties of what a Create returns.  Will assert if a failure.
 *
 * @param [in] testCase The LongBow test case (used for the clipboard)
 * @param [in] create The command create function pointer to test
 * @param [in] title The descriptive title to display in case of error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
testCommandCreate(const LongBowTestCase *testCase, MetisCommandOps * (*create)(MetisControlState * state), const char *title)
{
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = create(data->state);
    assertNotNull(ops, "%s: Got null ops", title);
    assertNotNull(ops->execute, "%s: Ops execute must not be null", title);
    assertNotNull(ops->command, "%s: Ops command must not be null", title);
    assertTrue(ops->closure == data->state, "%s: ops closure should be data->state, got wrong pointer", title);

    metisCommandOps_Destroy(&ops);
    assertNull(ops, "Ops not nulled by Destroy");
}

/**
 * Test a Help command's execution.
 *
 * A Help execution will display text (which we don't test).  We make sure there
 * is no memory leak and that it returns successfully.  We will call the passed create method
 * to create the Help command then execute its execute.
 *
 * @param [in] testCase The LongBow test case (used for the clipboard)
 * @param [in] create The command create function pointer to test
 * @param [in] title The descriptive title to display in case of error
 * @param [in] expected A MetisCommandReturn to use as the expected result
 *
 * Example:
 * @code
 * {
 *    // expectes MetisCommandReturn_Success
 *    testHelpExecute(testCase, metisControl_Add_Create, __func__, MetisCommandReturn_Success);
 *
 *    // expectes MetisCommandReturn_Exit
 *    testHelpExecute(testCase, metisControl_Quit_Create, __func__, MetisCommandReturn_Exit);
 * }
 * @endcode
 */
void
testHelpExecute(const LongBowTestCase *testCase, MetisCommandOps * (*create)(MetisControlState * state), const char *title, MetisCommandReturn expected)
{
    uint32_t beforeMemory = parcMemory_Outstanding();
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = create(data->state);
    MetisCommandReturn result = ops->execute(NULL, ops, NULL);
    assertTrue(result == expected, "Wrong return, got %d expected %d", result, expected);
    metisCommandOps_Destroy(&ops);
    uint32_t afterMemory = parcMemory_Outstanding();

    assertTrue(beforeMemory == afterMemory, "Memory leak by %d\n", (int) (afterMemory - beforeMemory));
}

/**
 * Verify that a list of commands is added by the Init function
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] testCase The LongBow test case (used for the clipboard)
 * @param [in] create We will create one of these and call it's init() function
 * @param [in] title The descriptive title to display in case of error
 * @param [in] commandList Null terminated list of commands
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
testInit(const LongBowTestCase *testCase, MetisCommandOps * (*create)(MetisControlState * state), const char *title, const char **commandList)
{
    // this will register 8 commands, so check they exist
    TestData *data = longBowTestCase_GetClipBoardData(testCase);
    MetisCommandOps *ops = create(data->state);
    assertNotNull(ops, "%s got null ops from the create function", title);
    assertNotNull(ops->init, "%s got null ops->init from the create function", title);

    ops->init(data->state->parser, ops);

    for (int i = 0; commandList[i] != NULL; i++) {
        bool success = metisCommandParser_ContainsCommand(data->state->parser, commandList[i]);
        assertTrue(success, "%s: Missing: %s", title, commandList[i]);
    }

    metisCommandOps_Destroy(&ops);
}
