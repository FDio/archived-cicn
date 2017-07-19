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

var LENGTH_MAIN = 350, // unused
    LENGTH_SERVER = 150, // unused
    LENGTH_SUB = 50, // unused
    WIDTH_SCALE = 2,
    EDGE_WIDTH_MIN = 2,
    EDGE_WIDTH_MID = 15
    EDGE_WIDTH_MAX = 35,
    BW_MID = 50,
    EDGE_RATE_MAX = 250000, // kbps
    COORDINATE_SCALE = 50,
    CACHE_RADIUS = 120,
    GREEN = 'green',
    MAX_COLOR_VALUE = 16777215
    MAX_BW_VALUE = 1200
    RED = '#C5000B',
    BLUE = '#0B00C5',
    ORANGE = 'orange',
    GRAY = 'gray',
    BLACK = '#2B1B17',
    CYAN = 'cyan',
    COLOR_RED = '#fa5858',
    COLOR_RED_SHADED = '#F8E0E0',
    COLOR_BLUE = '#0080ff',
    COLOR_BLUE_SHADED = '#CEE3F6',
    COLOR_GREEN = '#04B404',
    COLOR_GREEN_SHADED = '#E0F8E0';

var DIR = 'img/';

/* Network color scales */

var color_scale_lin = d3.scaleLinear().domain([0, 10, 20, 30, 40, 50]).range(['gray', 'green', 'blue', 'purple', 'black', 'yellow']);
var color_scale_log = d3.scaleLog().domain([50, 200, 1000]).range(['yellow', 'orange', 'red']);

DEFAULT_VIS_OPTIONS = {
    nodes: {
        size: 70,       // unused
      font:{color:'black'},
    scaling:{
          label: {
              min:0,
              max: 35 // unused// This defines the label size of the node
          }
        }
    },
    edges: {
    color: GRAY,
    smooth: false
  },
  physics:{
    barnesHut:{gravitationalConstant:-30000},
    stabilization: {iterations:2500}
  },
  groups: {
    'switch': {
        shape: 'triangle',
        color: '#FF9900' // orange
    },
    desktop: {
        shape: 'dot',
        color: "#2B7CE9" // blue
    },
    mobile: {
        shape: 'dot',
        color: "#5A1E5C" // purple
    },
    server: {
        shape: 'square',
        color: "#C5000B" // red
    },
    internet: {
        shape: 'square',
        color: "#109618" // green
    }
  }
}

var Network = Class.extend({

    /**
     * @brief Class constructor
     */
    init: function(id)
    {

      this._id = id;
      this._server = new ServerEventsDispatcher(URL);

      this._server.bind('insert', 'local.connection', this.on_connect.bind(this));
      this._server.bind("insert", "node", this.on_insert_node.bind(this));
      this._server.bind("insert", "channel", this.on_insert_channel.bind(this));
      this._server.bind("update", "node", this.on_update_node.bind(this));
      this._server.bind("update", "channel", this.on_update_channel.bind(this));

      this._map_interface_node = new Object();
      this._map_id_type = new Object();

      var body = document.body,
      html = document.documentElement;

      var HEIGTH = Math.max( body.scrollHeight, body.offsetHeight,
                         html.clientHeight, html.scrollHeight, html.offsetHeight );
      var WIDTH = Math.max( body.scrollWidth, body.offsetWidth,
                         html.clientWidth, html.scrollWidth, html.offsetWidth );

      this._nodes = new vis.DataSet({});
      this._edges = new vis.DataSet({});
      this._edgesInfo = [];

      this._map_id_name = new Object();
      this._map_name_id = new Object();

      this._pending_edges = Array();
      this._pending_nodes = Array();

      var container = document.getElementById(id);
      var data = {
          nodes: this._nodes,
          edges: this._edges
      };

      vis.Network.prototype.setScale = function (scale) {
          var options = {
              nodes: []
          };
          var animationOptions = {
              scale: scale,
              animation: options.animation
          };
          this.view.moveTo(animationOptions);
      };

      this._network = new vis.Network(container, data, DEFAULT_VIS_OPTIONS);
      window.onresize = this.do_fit.bind(this);

      this.intervalID = setInterval(
         (function(self) {         //Self-executing func which takes 'this' as self
             return function() {   //Return a function in the context of 'self'
                self._tick();
             }
         })(this),
        500);
    },

    _tick: function()
    {
      this._edges.update(this._pending_edges);
      this._pending_edges.length = 0;

      this._nodes.update(this._pending_nodes);
      this._pending_nodes.length = 0;
    },

    do_fit: function()
    {
      this._network.fit();
    },

    on_connect: function()
    {
        // Request nodes with interfaces, and later, links
        var node_query = new Query('select', 'node');
        this._server.send(node_query.to_dict());
    },

    on_insert_node: function(query)
    {
        data = query.params
        // Update mac->node map
        if (data.interfaces) {
            for (var i=0; i<data.interfaces.length; i++) {
                this._map_interface_node[data.interfaces[i]] = data.id;
            }
        }

        // RIGHT PLOTS
        this._map_id_name[data.id] = data.name;
        this._map_name_id[data.name] = data.id;

        font = new Object();
        font.size = 40;
        font.color = 'black';
        //font.background = 'black';
        font.strokeColor = 'white';
        font.strokeWidth = 2;
        font.align = 'top';
        //font.vadjust = -10;

        // Update node
        var node = new Object();
        node.id = data.id;
        node.font = font;

        node.font.color = '#000000';

        node.shape = 'image'

        node.x = data.x * COORDINATE_SCALE;
        node.y = data.y * COORDINATE_SCALE;
        node.size = 0.75 * COORDINATE_SCALE;

        node.image = DIR + data.category + '.png';

        node.physics = false;
        node.has_producer = false; // FIB entry
        if (data.groups.length > 0) {
            this._nodes.add(node);
        }
        if (query.last) {
            this.do_fit();
            var link_query = new Query('select', 'channel');
            this._server.send(link_query.to_dict());
        }
    },

    on_update_node: function(query)
    {
        var data = query.params;
        var node = new Object();

        var filter_dict = {};
        query.filter.forEach(function(filter) {
            [key, op, value] = filter;
            if (op == '==') {
                filter_dict[key] = value;
            }
        });

        if (filter_dict.id === undefined) {
            if (filter_dict.name === undefined) {
                console.log("Wrong update query");
                return;
            }
            node.id = this._map_name_id[filter_dict.name];
        } else {
            node.id = filter_dict.id;
        }
        node.image = DIR + data.category + '.png';

        this._pending_nodes.push(node);
    },

    on_insert_channel: function(query)
    {
        data = query.params;

        font = new Object();
        font.size = 35;
        font.color = 'black';
        //font.background = 'black';
        font.strokeColor = 'white';
        font.strokeWidth = 2;
        font.align = 'top';
        font.vadjust = -20;

        if ((data.type.indexOf('wiredchannel') !== -1) || (data.type.indexOf('link') !== -1) || (data.type.indexOf('phylink') !== -1) || (data.type.indexOf('memiflink') !== -1)) {
            var edgeu = new Object();
            edgeu.id = data.id;
            edgeu.width = EDGE_WIDTH_MIN;
            edgeu.color = GRAY
            edgeu.fontColor = RED;
            edgeu.font = font;

            if ((data.type.indexOf('link') !== -1) || (data.type.indexOf('memiflink') !== -1)) {
                edgeu.from = data.src_node;
                edgeu.to = data.dst_node;
            } else {
                edgeu.from = this._map_interface_node[data.src];
                edgeu.to   = this._map_interface_node[data.dst];

            }

            //this._edges.update(edgeu);
            this._pending_edges.push(edgeu);
            this._edgesInfo[edgeu.id] =  data.capacity;

        } else if (data.type.indexOf('emulatedchannel') !== -1) {
            if (data.stations !== undefined) {
              data.stations.forEach(function(station) {
                  var edgeu = new Object();
                  station_name = this._map_id_name[station];
                  edgeu.id = 'tap-' + station_name + '-' + data.name;
                  edgeu.width = EDGE_WIDTH_MIN;
                  edgeu.color = GRAY;
                  edgeu.fontColor = RED;

                  edgeu.from = station;
                  edgeu.to = data.ap;
                  edgeu.dashes = [10,10];
                  edgeu.font = font;
                  this._pending_edges.push(edgeu);

                  this._edgesInfo[edgeu.id] =  data.capacity;
              }.bind(this));
            }
        }
    },

    on_update_channel: function(query)
    {
        data = query.params;
        if (data.bw_upstream === undefined)
            return;

        var id = query.filter[0][2];
        var bw = parseFloat(Math.round((parseFloat(data.bw_upstream) + parseFloat(data.bw_downstream)) / 1024 / 1024 * 8 * 10) /10).toFixed(1);

        var edgeu = new Object();
        edgeu.id = id;
        if (bw < BW_MID) {
            // 0 MIN
            // x  ??
            // BW_MID EDGE_WIDTH_MID
            edgeu.color = color_scale_lin(bw);
            edgeu.width = (EDGE_WIDTH_MID * bw) / BW_MID;
        } else {
            // BW_MID  0 -> 0                               + EDGE_WIDTH_MID
            // log() = 0 -> 0                               + EDGE_WIDTH_MID
            //       = x -> ??
            // log() = 4 -> EDGE_WIDTH_MAX - EDGE_WIDTH_MID + EDGE_WIDTH_MID
            edgeu.color = color_scale_log(bw);
            edgeu.width = EDGE_WIDTH_MID + Math.log10(bw - BW_MID) * (EDGE_WIDTH_MAX - EDGE_WIDTH_MID) / 4
        }

        if (bw != 0.0) {
            if (bw > 1000)
                edgeu.label = Math.round(bw/10)/100 + " Gbps";
            else
                edgeu.label = bw + " Mbps";
        } else {
            edgeu.label = "";
        }


        this._pending_edges.push(edgeu);

    },

});
