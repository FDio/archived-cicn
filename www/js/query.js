/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 *
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
 *
 */

var Query = Class.extend({
  /**
   * @brief Query class
   * @param action (string) :
   * @param object_name (string) :
	 * @param filter (list[tuple(key, op, value)]) :
   * @param params (dict[field_name, value]) :
   * @param field_names (list[string])
   * @return : a Query instance
   */
  init: function(action, object_name, filter = null, params = null, field_names = null, last = false)
  {
    this._action = action;
    this._object_name = object_name;
    this._filter = filter;
    this._params = params;
    this._field_names = field_names;
    this._last = last;
  },

  /**
   * @brief : dict representation of a Query
   * @return : a dict representing the Query
   */
  to_dict: function()
  {
      query_dict = {
        'action': this._action,
        'object_name': this._object_name,
        'filter': this._filter,
        'params': this._params,
        'fields': this._fields,
        'last': this._last
      };
      return query_dict;
  }

});
