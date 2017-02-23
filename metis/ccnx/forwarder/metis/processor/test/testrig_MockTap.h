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


#ifndef Metis_testrig_MockTap_h
#define Metis_testrig_MockTap_h

// =========================================================================
// Mock for tap testing
// Allows the test to set the IsTapOnX return values.
// Counts the number of calls to each TapOnX.
// Records the last message pointer
// The user sets and examines values in the static "testTap" variable and
// passes "testTapTemplate" to the tap setup.

static bool testTap_IsTapOnReceive(const MetisTap *tap);
static bool testTap_IsTapOnSend(const MetisTap *tap);
static bool testTap_IsTapOnDrop(const MetisTap *tap);
static void testTap_TapOnReceive(MetisTap *tap, const MetisMessage *message);
static void testTap_TapOnSend(MetisTap *tap, const MetisMessage *message);
static void testTap_TapOnDrop(MetisTap *tap, const MetisMessage *message);

// this test variable is zeroed in each FIXTURE_SETUP.
// To test tap functionality, set the various callOnX flags, run your test,
// then check the onXCounts to make sure they are right.
struct testTap_s {
    bool callOnReceive;
    bool callOnSend;
    bool callOnDrop;
    unsigned onReceiveCount;
    unsigned onSendCount;
    unsigned onDropCount;

    const MetisMessage *lastMessage;
} testTap;

// you should not need tochange this template
MetisTap testTapTemplate = {
    .context        = &testTap,
    .isTapOnReceive = &testTap_IsTapOnReceive,
    .isTapOnSend    = &testTap_IsTapOnSend,
    .isTapOnDrop    = &testTap_IsTapOnDrop,
    .tapOnReceive   = &testTap_TapOnReceive,
    .tapOnSend      = &testTap_TapOnSend,
    .tapOnDrop      = &testTap_TapOnDrop
};

static bool
testTap_IsTapOnReceive(const MetisTap *tap)
{
    struct testTap_s *mytap = (struct testTap_s *) tap->context;
    return mytap->callOnReceive;
}

static bool
testTap_IsTapOnSend(const MetisTap *tap)
{
    struct testTap_s *mytap = (struct testTap_s *) tap->context;
    return mytap->callOnSend;
}

static bool
testTap_IsTapOnDrop(const MetisTap *tap)
{
    struct testTap_s *mytap = (struct testTap_s *) tap->context;
    return mytap->callOnDrop;
}

static void
testTap_TapOnReceive(MetisTap *tap, const MetisMessage *message)
{
    struct testTap_s *mytap = (struct testTap_s *) tap->context;
    mytap->onReceiveCount++;
    mytap->lastMessage = message;
}

static void
testTap_TapOnSend(MetisTap *tap, const MetisMessage *message)
{
    struct testTap_s *mytap = (struct testTap_s *) tap->context;
    mytap->onSendCount++;
    mytap->lastMessage = message;
}

static void
testTap_TapOnDrop(MetisTap *tap, const MetisMessage *message)
{
    struct testTap_s *mytap = (struct testTap_s *) tap->context;
    mytap->onDropCount++;
    mytap->lastMessage = message;
}
#endif // Metis_testrig_MockTap_h
