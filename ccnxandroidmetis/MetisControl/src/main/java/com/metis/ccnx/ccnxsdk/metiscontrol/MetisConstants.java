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

package com.metis.ccnx.ccnxsdk.metiscontrol;

public class MetisConstants {

    public static final int MetisDefaultListenerPort = 9695;

    public static final String MetisModule_Control = "Control";
    public static final String MetisModule_FIB = "FIB";
    public static final String MetisModule_PIT = "PIT";
    public static final String MetisModule_ContentStore = "ContentStore";
    public static final String MetisModule_TransportLinkAdapter = "TransportLinkAdapter";

    public static final String CCNxNameMetis_Forwarder = "ccnx:/local/forwarder";
    public static final String CCNxNameMetis_Control = CCNxNameMetis_Forwarder + "/" + MetisModule_Control;
    public static final String CCNxNameMetis_FIB = CCNxNameMetis_Forwarder + "/" + MetisModule_FIB;
    public static final String CCNxNameMetis_PIT = CCNxNameMetis_Forwarder + "/" + MetisModule_PIT;
    public static final String CCNxNameMetis_ContentStore = CCNxNameMetis_Forwarder + "/" + MetisModule_ContentStore;
    ;
    public static final String CCNxNameMetis_Link = CCNxNameMetis_Forwarder + "/" + MetisModule_TransportLinkAdapter;

    // General Commands
    public static final int MetisCommandSegment = 3;
    public static final String MetisCommand_Lookup = "lookup";
    public static final String MetisCommand_Add = "add";
    public static final String MetisCommand_List = "list";
    public static final String MetisCommand_Remove = "remove";
    public static final String MetisCommand_Resize = "resize";
    public static final String MetisCommand_Set = "set";
    public static final String MetisCommand_Quit = "quit";
    public static final String MetisCommand_Run = "spawn";
    public static final String MetisCommand_Stats = "stats";

    public static final String MetisCommand_LogLevel = "level";
    public static final String MetisCommand_LogDebug = "debug";
    public static final String MetisCommand_LogInfo = "info";
    public static final String MetisCommand_LogError = "error";
    public static final String MetisCommand_LogAll = "all";
    public static final String MetisCommand_LogOff = "off";
    public static final String MetisCommand_LogNotice = "notice";
    ;

    // Module Specific Commands
    public static final String CCNxNameMetisCommand_LinkConnect = CCNxNameMetis_Link + "/" + MetisCommand_Add;   // create a connection to interface specified in payload, returns name
    public static final String CCNxNameMetisCommand_LinkDisconnect = CCNxNameMetis_Link + "/" + MetisCommand_Remove; // remove a connection to interface specified in payload, by name
    public static final String CCNxNameMetisCommand_LinkList = CCNxNameMetis_Link + "/" + MetisCommand_List;     // list interfaces

    public static final String CCNxNameMetisCommand_FIBLookup = CCNxNameMetis_FIB + "/" + MetisCommand_Lookup;   // return current FIB contents for name in payload
    public static final String CCNxNameMetisCommand_FIBList = CCNxNameMetis_FIB + "/" + MetisCommand_List;       // list current FIB contents
    public static final String CCNxNameMetisCommand_FIBAddRoute = CCNxNameMetis_FIB + "/" + MetisCommand_Add;    // add route for arguments in payload
    public static final String CCNxNameMetisCommand_FIBRemoveRoute = CCNxNameMetis_FIB + "/" + MetisCommand_Remove;                  // remove route for arguments in payload

    public static final String CCNxNameMetisCommand_PITLookup = CCNxNameMetis_PIT + "/" + MetisCommand_Lookup;   // return current PIT contents for name in payload
    public static final String CCNxNameMetisCommand_PITList = CCNxNameMetis_PIT + "/" + MetisCommand_List;       // list current PIT contents

    public static final String CCNxNameMetisCommand_ContentStoreResize = CCNxNameMetis_ContentStore + "/" + MetisCommand_Resize;         // resize current content store to size in MB in payload

    public static final String CCNxNameMetisCommand_Quit = CCNxNameMetis_Control + "/" + MetisCommand_Quit;      // ask the forwarder to exit
    public static final String CCNxNameMetisCommand_Run = CCNxNameMetis_Control + "/" + MetisCommand_Run;        // start a new forwarder instance
    public static final String CCNxNameMetisCommand_Set = CCNxNameMetis_Control + "/" + MetisCommand_Set;        // set a forwarder variable
    public static final String CCNxNameMetisCommand_Stats = CCNxNameMetis_Control + "/" + MetisCommand_Stats;    // get forwarder stats
};
