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
 * The MetisForwarder supports a Tap that will inspect all messages passing through the
 * forwarder.  See metisForwarder_AddTap() and metisForwarder_RemoveTap().
 *
 *
 * Example:
 * @code
 * {
 *     struct testTap_s {
 *        bool callOnReceive;
 *        unsigned onReceiveCount;
 *     } testTap;
 *
 *     static bool
 *     testTap_IsTapOnReceive(const MetisTap *tap)
 *     {
 *        struct testTap_s *mytap = (struct testTap_s *) tap->context;
 *        return mytap->callOnReceive;
 *     }
 *
 *     static void
 *     testTap_TapOnReceive(MetisTap *tap, const MetisMessage *message)
 *     {
 *        struct testTap_s *mytap = (struct testTap_s *) tap->context;
 *        mytap->onReceiveCount++;
 *        mytap->lastMessage = message;
 *     }
 *
 *     MetisTap testTapTemplate = {
 *        .context        = &testTap,
 *        .isTapOnReceive = &testTap_IsTapOnReceive,
 *        .isTapOnSend    = NULL,
 *        .isTapOnDrop    = NULL,
 *        .tapOnReceive   = &testTap_TapOnReceive,
 *        .tapOnSend      = NULL,
 *        .tapOnDrop      = NULL
 *     };
 *
 * }
 * @endcode
 *
 */

#ifndef Metis_metis_Tap_h
#define Metis_metis_Tap_h

struct metis_tap;


typedef struct metis_tap MetisTap;

/**
 * Defines callbacks for message taps
 *
 * Each of the taps (tapOnReceive, tapOnSend, tapOnDrop) may be NULL.
 *   if a tap is not null, then the correspnoding isX function must be non-null.  The isX functions
 *   allow turning on/off particular calls depending on user preference.
 */
struct metis_tap {

    /**
     * A user-defined parameter
     */
    void *context;

    /**
     * Determines if the tapOnReceive() function should be called
     *
     * If *tapOnReceive is non-null, this function must be defined too.
     *
     * @param [in] MetisTap The tap structure
     *
     * @return true call the tap function
     * @return false Do not call the tap function.
     */
    bool (*isTapOnReceive)(const MetisTap *tap);

    /**
     * Determines if the tapOnSend() function should be called
     *
     * If *tapOnSend is non-null, this function must be defined too.
     *
     * @param [in] MetisTap The tap structure
     *
     * @return true call the tap function
     * @return false Do not call the tap function.
     */
    bool (*isTapOnSend)(const MetisTap *tap);

    /**
     * Determines if the tapOnDrop() function should be called
     *
     * If *tapOnDrop is non-null, this function must be defined too.
     *
     * @param [in] MetisTap The tap structure
     *
     * @return true call the tap function
     * @return false Do not call the tap function.
     */
    bool (*isTapOnDrop)(const MetisTap *tap);

    /**
     * Called for each message entering the message processor.  May be NULL.
     *
     * @param [in] MetisTap The tap structure
     */
    void (*tapOnReceive)(MetisTap *tap, const MetisMessage *message);

    /**
     * Called for each message forwarded by the message processor.  May be NULL.
     *
     * @param [in] MetisTap The tap structure
     */
    void (*tapOnSend)(MetisTap *tap, const MetisMessage *message);

    /**
     * Called for each message dropped by the message processor.  May be NULL.
     *
     * @param [in] MetisTap The tap structure
     */
    void (*tapOnDrop)(MetisTap *tap, const MetisMessage *message);
};


#endif // Metis_metis_Tap_h
