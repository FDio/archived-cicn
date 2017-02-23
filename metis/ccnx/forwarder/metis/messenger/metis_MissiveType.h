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
 * @file metis_MissiveType
 * @brief Defines what a Missive represents
 *
 * Currently, missives only carry information about the state of a connection
 * (created, up, down, closed, destroyed).
 *
 */

#ifndef Metis_metis_MissiveType_h
#define Metis_metis_MissiveType_h

/**
 * @typedef Represents the state of a connection
 * @abstract CREATE is the initial state.  UP & DOWN are recurrent states.  CLOSED is transient.  DESTROYED is the terminal state.
 * @constant MetisMissiveType_ConnectionCreate    Connection created (new)
 * @constant MetisMissiveType_ConnectionUp        Connection is active and passing data
 * @constant MetisMissiveType_ConnectionDown      Connection is inactive and cannot pass data
 * @constant MetisMissiveType_ConnectionClosed    Connection closed and will be destroyed
 * @constant MetisMissiveType_ConnectionDestroyed Connection destroyed
 * @discussion State transitions:
 *                initial   -> CREATE
 *                CREATE    -> (UP | DOWN)
 *                UP        -> (DOWN | DESTROYED)
 *                DOWN      -> (UP | CLOSED | DESTROYED)
 *                CLOSED    -> DESTROYED
 *                DESTROYED -> terminal
 */
typedef enum {
    MetisMissiveType_ConnectionCreate,
    MetisMissiveType_ConnectionUp,
    MetisMissiveType_ConnectionDown,
    MetisMissiveType_ConnectionClosed,
    MetisMissiveType_ConnectionDestroyed
} MetisMissiveType;
#endif // Metis_metis_MissiveType_h
