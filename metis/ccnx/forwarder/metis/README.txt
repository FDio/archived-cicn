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

=================
Metis (mythology)
Metis was the the Titaness of wisdom and deep thought. The Greek word
"metis" meant a quality that combined wisdom and cunning.
=================

Metis is the CCNx 1.0 forwarder.  It uses a modular design so adding new
encapsulations, forwarding behavior, or event dispatching is
well-defined and has minimum (or no) side effects.

=================
Building

"make" will create "libmetis.a" and "metis".  The library is used by other
parts of Libccnx, such as Transport testing and the mobile builds.  The
executable "metis" is for command-line invocation of a forwarding daemon.

Metis uses a few things from parc/algol and ccnx/common.

Metis uses the LongBow XUnit test framework. "make check" will run the tests.

Metis is built with Libevent 2.0 for the dispatcher.  Libevent is visible in the
code in the following modules.  To use a different event dispatcher, these modules
would need to be updated: metisDispatcher.c, metisStreamBuffer.c, metisMessage.c,
metis_StreamConnection.c

=================
Code Layout

Each directory has its code and headers along with a test/ subdirectory with
LongBow unit tests.

config/
config/test
- Forwarder configuration, from a file or CLI or web
  The default telnet port is 2001.
  The default web port is 2002.

core/
core/test
- The forwarder (metisForwarder) that bundles the whole thing and the event
  dispatcher (metisDispatcher).  Also includes various utility classes, such as
  MetisMessage that wraps all messages inside the forwarder.

io/
io/test/
- I/O related primatives.  These include "listeners" that monitor for specific types
  of incoming packets.  io/ includes tunnels, ip overlays, and direct L2 encapsulation.

messenger/
messenger/test/
- event messages within the router, such as notificaitons when an interface
  comes up or goes down.

platforms/
- platform specific functions, such as dealing with interfaces and
  direct Ethernet encapsulation.  The files in here implement function
  prototypes (usually in core/ or io/), and are tested with unit tests of those
  prototypes.  So, there is no platforms/test/ directory.

processor/
processor/test/
- The message processor.  Forwards Interests and objects.

strategies/
strategies/test/
- Forwarding strategies.  Each FIB entry indicates the strategy that should
  be used to forward an interest among its entries.

libmetis.a
- The output library

test/
- These are system-level tests

