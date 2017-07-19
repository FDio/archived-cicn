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

var Main = Class.extend({

    /**
     * @brief Class constructor
     */
    init: function()
    {
      this._server = new ServerEventsDispatcher(URL);
      this._server.bind('insert', 'local.connection', this.on_connect.bind(this));
      this._network = new Network('network');
    },

    on_connect: function()
    {
        $('#cnxled').removeClass('led-red').addClass('led-green');
    },

});

$(function() {
  main = new Main();
});
