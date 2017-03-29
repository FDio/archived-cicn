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

package com.iget.ccnx.service;

public class CCNxService {
  /**
   * Loading of IGet Native libraries.
   */
  static {

    System.loadLibrary("crystax");
    System.loadLibrary("iget-wrapper");
    System.loadLibrary("consumer-producer");
    System.loadLibrary("gnustl_shared");
    System.loadLibrary("ccnx_api_portal");
    System.loadLibrary("ccnx_transport_rta");
    System.loadLibrary("ccnx_api_control");
    System.loadLibrary("ccnx_api_notify");
    System.loadLibrary("ccnx_common");
    System.loadLibrary("parc");
    System.loadLibrary("longbow");
    System.loadLibrary("longbow-textplain");
    System.loadLibrary("longbow-ansiterm");
    System.loadLibrary("ssl");
    System.loadLibrary("crypto");
  }



  public native static void
  startIGet(String downloadPath, String destinationPath);

}
