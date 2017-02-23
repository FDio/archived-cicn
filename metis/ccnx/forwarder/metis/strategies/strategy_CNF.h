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
 * Conjunctive Normal Form forwarding.  Nexthops are a conjunctive set of
 * disjunctive sets of interface IDs.  That is, at the first conjunctive
 * level, we have:
 *   A_1 and A_2 and A_3 and ... and A_m
 *
 * Each set A_i is made up of a disjunctive set:
 *   B_(i,1) or B_(i,2) or ... or B_(i,n)
 *
 * An interest is forwarded to every conjunctive set A_i.  If a set A_i has
 * more than one B_(i,j), we pick a B_(i,j) using a weighted round robin, but only
 * one B_(i,j) is used.
 *
 * The final set of egress interfaces is the union of B_(i,j) over all i.
 *
 *
 * THIS STRATEGY IS NOT IMPLEMENTED
 */

#ifndef Metis_strategy_CNF_h
#define Metis_strategy_CNF_h

#include <ccnx/forwarder/metis/strategy/metis_Strategy.h>
#endif // Metis_strategy_CNF_h
