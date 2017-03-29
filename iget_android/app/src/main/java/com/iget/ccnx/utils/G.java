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

package com.iget.ccnx.utils;

import android.util.Log;

/**
 * Global convenience class used for NFD Service wide constants and logs.
 *
 * If log messages need to be written to persistent storage, this is the
 * place to implement it.
 *
 */
public class G {

  /** Flag that turns on/off debugging log output. */
  private static final boolean DEBUG = true;

  /** Tag used in log output to identify NFD Service. */
  private static final String TAG = "NFDService";

  /**
   * Designated log message method that provides flexibility in message logging.
   *
   * @param tag Tag to identify log message.
   * @param format Format qualifiers as used in String.format()
   * @param args Output log message.
   */
  public static void Log(String tag, String format, Object ... args) {
    if (DEBUG) {
      Log.d(tag, String.format(format, args));
    }
  }

  /**
   * Convenience method to log a message with a specified tag.
   *
   * @param tag Tag to identify log message.
   * @param message Output log message.
   */
  public static void Log(String tag, String message) {
    Log(tag, "%s", message);
  }

  /**
   * Convenience method to log messages with the default tag.
   *
   * @param message Output log message.
   */
  public static void Log(String message) {
    Log(TAG, message);
  }

  /**
   * Gets the tag in which logs are posted with.
   *
   * @return TAG that is used by this log class.
   */
  public static String getLogTag() {
    return TAG;
  }
}
