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

#include <config.h>
#include <LongBow/unit-test.h>

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../parc_Base64.c"
#include <parc/algol/parc_SafeMemory.h>

LONGBOW_TEST_RUNNER(parc_Base64)
{
    // The following Test Fixtures will run their corresponding Test Cases.
    // Test Fixtures are run in the order specified, but all tests should be idempotent.
    // Never rely on the execution order of tests or share state between them.
    LONGBOW_RUN_TEST_FIXTURE(Global);
    LONGBOW_RUN_TEST_FIXTURE(Local);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(parc_Base64)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_Base64)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parcBase64_Decode);
    LONGBOW_RUN_TEST_CASE(Global, parcBase64_Decode_Linefeeds);
    LONGBOW_RUN_TEST_CASE(Global, parcBase64_Encode);
    LONGBOW_RUN_TEST_CASE(Global, parcBase64_Encode_Binary);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

static struct testvector_s {
    char *plaintext;
    char *encoded;
} testvector[] = {
    // Test vectors from RFC 4648
    { .plaintext = "",       .encoded = ""         },
    { .plaintext = "f",      .encoded = "Zg=="     },
    { .plaintext = "fo",     .encoded = "Zm8="     },
    { .plaintext = "foo",    .encoded = "Zm9v"     },
    { .plaintext = "foob",   .encoded = "Zm9vYg==" },
    { .plaintext = "fooba",  .encoded = "Zm9vYmE=" },
    { .plaintext = "foobar", .encoded = "Zm9vYmFy" },
    { .plaintext = NULL,     .encoded = NULL       }
};

LONGBOW_TEST_CASE(Global, parcBase64_Decode)
{
    int i = 0;
    while (testvector[i].plaintext != NULL) {
        PARCBufferComposer *input = parcBufferComposer_Create();
        PARCBufferComposer *truth = parcBufferComposer_Create();
        PARCBufferComposer *output = parcBufferComposer_Create();

        parcBufferComposer_PutString(input, testvector[i].encoded);
        PARCBuffer *inputBuffer = parcBufferComposer_ProduceBuffer(input);
        parcBufferComposer_PutString(truth, testvector[i].plaintext);
        PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

        parcBase64_Decode(output, parcBufferComposer_GetBuffer(input));
        PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);

        assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
                   "encoding, expected '%s' got '%s'",
                   parcBuffer_ToHexString(truthBuffer),
                   parcBuffer_ToHexString(outputBuffer));

        i++;

        parcBuffer_Release(&inputBuffer);
        parcBuffer_Release(&truthBuffer);
        parcBuffer_Release(&outputBuffer);

        parcBufferComposer_Release(&input);
        parcBufferComposer_Release(&output);
        parcBufferComposer_Release(&truth);
    }
}

LONGBOW_TEST_CASE(Global, parcBase64_Decode_Linefeeds)
{
    PARCBufferComposer *input = parcBufferComposer_Create();
    PARCBufferComposer *truth = parcBufferComposer_Create();
    PARCBufferComposer *output = parcBufferComposer_Create();

    char plaintext[] = "It was a dark and stormy night, and all through the code not bit was stirring.\x0A";
    char encoded_with_crlf[] = "SXQg" "\x0D\x0A" "d2FzIGEgZGFyayBhbmQgc3Rvcm15IG5pZ2h0LCBhbmQgYWxsIHRocm91Z2gg" "\x0D\x0A" "dGhlIGNvZGUgbm90IGJpdCB3YXMgc3RpcnJpbmcuCg==";

    parcBufferComposer_PutString(input, encoded_with_crlf);
    PARCBuffer *inputBuffer = parcBufferComposer_ProduceBuffer(input);

    parcBufferComposer_PutString(truth, plaintext);
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    parcBase64_Decode(output, parcBufferComposer_GetBuffer(input));
    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);
    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "encoding, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&inputBuffer);
    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&input);
    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}


LONGBOW_TEST_CASE(Global, parcBase64_Encode)
{
    int i = 0;
    while (testvector[i].plaintext != NULL) {
        PARCBufferComposer *input = parcBufferComposer_Create();
        PARCBufferComposer *truth = parcBufferComposer_Create();
        PARCBufferComposer *output = parcBufferComposer_Create();

        parcBufferComposer_PutString(input, testvector[i].plaintext);
        PARCBuffer *inputBuffer = parcBufferComposer_ProduceBuffer(input);
        parcBufferComposer_PutString(truth, testvector[i].encoded);
        PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

        parcBase64_Encode(output, parcBufferComposer_GetBuffer(input));
        PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);
        assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
                   "encoding, expected '%s' got '%s'",
                   parcBuffer_ToHexString(truthBuffer),
                   parcBuffer_ToHexString(outputBuffer));

        i++;

        parcBuffer_Release(&inputBuffer);
        parcBuffer_Release(&truthBuffer);
        parcBuffer_Release(&outputBuffer);

        parcBufferComposer_Release(&input);
        parcBufferComposer_Release(&output);
        parcBufferComposer_Release(&truth);
    }
}

LONGBOW_TEST_CASE(Global, parcBase64_Encode_Binary)
{
    uint8_t zero[] = { 0 };

    //                       1       2       3       4           5           6           7
    char *truthvector[] = { "AA==", "AAA=", "AAAA", "AAAAAA==", "AAAAAAA=", "AAAAAAAA", "AAAAAAAAAA==" };

    for (int i = 0; i < sizeof(truthvector) / sizeof(truthvector[0]); i++) {
        PARCBufferComposer *input = parcBufferComposer_Create();
        PARCBufferComposer *truth = parcBufferComposer_Create();
        PARCBufferComposer *output = parcBufferComposer_Create();

        parcBufferComposer_PutArray(truth, (uint8_t *) truthvector[i], strlen(truthvector[i]));
        PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

        // just keep apending a zero to make it longer
        parcBuffer_SetLimit(parcBufferComposer_GetBuffer(input), parcBuffer_Capacity(parcBufferComposer_GetBuffer(input)));
        parcBuffer_SetPosition(parcBufferComposer_GetBuffer(input), i);
        parcBufferComposer_PutArray(input, zero, 1);
        PARCBuffer *inputBuffer = parcBufferComposer_ProduceBuffer(input);

        parcBase64_Encode(output, parcBufferComposer_GetBuffer(input));
        PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);

        assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
                   "encoding, expected '%s' got '%s'",
                   parcBuffer_ToHexString(truthBuffer),
                   parcBuffer_ToHexString(outputBuffer));

        parcBuffer_Release(&inputBuffer);
        parcBuffer_Release(&truthBuffer);
        parcBuffer_Release(&outputBuffer);

        parcBufferComposer_Release(&input);
        parcBufferComposer_Release(&output);
        parcBufferComposer_Release(&truth);
    }
}

LONGBOW_TEST_FIXTURE(Local)
{
    LONGBOW_RUN_TEST_CASE(Local, encodeWithPad_0);
    LONGBOW_RUN_TEST_CASE(Local, encodeWithPad_1);
    LONGBOW_RUN_TEST_CASE(Local, encodeWithPad_2);
    LONGBOW_RUN_TEST_CASE(Local, decode_invalid);
    LONGBOW_RUN_TEST_CASE(Local, decode_1);
    LONGBOW_RUN_TEST_CASE(Local, decode_2);
    LONGBOW_RUN_TEST_CASE(Local, decode_3);
}

LONGBOW_TEST_FIXTURE_SETUP(Local)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Local)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDOUT_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks %d memory allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

/**
 * This will encode "foo"
 */
LONGBOW_TEST_CASE(Local, encodeWithPad_0)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "foobar";
    PARCBufferComposer *truth = parcBufferComposer_Create();
    parcBufferComposer_PutString(truth, "Zm9v");
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    _encodeWithPad(output, input, 0);
    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);
    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "Failed 3-byte encode, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}

/**
 * This will encode "fo" because we tell it there's 1 pad byte
 */
LONGBOW_TEST_CASE(Local, encodeWithPad_1)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "foobar";
    PARCBufferComposer *truth = parcBufferComposer_Create();
    parcBufferComposer_PutString(truth, "Zm8=");
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    _encodeWithPad(output, input, 1);
    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);

    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "Failed 3-byte encode, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}

/**
 * This will encode "f" because we tell it there's 2 pad byte
 */
LONGBOW_TEST_CASE(Local, encodeWithPad_2)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "foobar";
    PARCBufferComposer *truth = parcBufferComposer_Create();
    parcBufferComposer_PutString(truth, "Zg==");
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    _encodeWithPad(output, input, 2);
    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);

    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "Failed 3-byte encode, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}


LONGBOW_TEST_CASE(Local, decode_1)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "Zg==";
    PARCBufferComposer *truth = parcBufferComposer_Create();
    parcBufferComposer_PutString(truth, "f");
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    bool success = _decode(output, input);
    assertTrue(success, "Valid base64 failed decode");

    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);
    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "Failed 3-byte encode, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}

LONGBOW_TEST_CASE(Local, decode_2)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "Zm8=";
    PARCBufferComposer *truth = parcBufferComposer_Create();
    parcBufferComposer_PutString(truth, "fo");
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    bool success = _decode(output, input);
    assertTrue(success, "Valid base64 failed decode");

    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);

    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "Failed 3-byte encode, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}

LONGBOW_TEST_CASE(Local, decode_3)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "Zm9v";
    PARCBufferComposer *truth = parcBufferComposer_Create();
    parcBufferComposer_PutString(truth, "foo");
    PARCBuffer *truthBuffer = parcBufferComposer_ProduceBuffer(truth);

    bool success = _decode(output, input);
    assertTrue(success, "Valid base64 failed decode");
    PARCBuffer *outputBuffer = parcBufferComposer_ProduceBuffer(output);

    assertTrue(parcBuffer_Equals(truthBuffer, outputBuffer),
               "Failed 3-byte encode, expected '%s' got '%s'",
               parcBuffer_ToHexString(truthBuffer),
               parcBuffer_ToHexString(outputBuffer));

    parcBuffer_Release(&truthBuffer);
    parcBuffer_Release(&outputBuffer);

    parcBufferComposer_Release(&output);
    parcBufferComposer_Release(&truth);
}

LONGBOW_TEST_CASE(Local, decode_invalid)
{
    PARCBufferComposer *output = parcBufferComposer_Create();
    uint8_t input[] = "@@@@";

    bool success = _decode(output, input);
    assertFalse(success, "Invalid base64 somehow decoded");

    parcBufferComposer_Release(&output);
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_Base64);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
