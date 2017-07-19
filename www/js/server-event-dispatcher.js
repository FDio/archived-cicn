/* Inspired from https://gist.github.com/ismasan/299789 (MIT licensed) */
$(function() {
  ServerEventsDispatcher = function(url){
    var callbacks = {};
    var is_open = false;
    var timeout = null;
		var timeoutInterval = 2; /* s */
		var self = this;
    var ws = null;

		this.open = function(){
			ws = new WebSocket(url);
      ws.binaryType = "arraybuffer";

      // dispatch to the right handlers
      ws.onmessage = function(evt){
        var json = JSON.parse(evt.data)
        //console.log("EVENT", json);
        dispatch(json);
      };

      ws.onclose = function(){
        is_open = false;
        dispatch({action: 'delete', object_name: 'local.connection'})

        timeout = setTimeout(function() { self.open(); }, self.timeoutInterval);
      }

      ws.onopen = function() {
        if (timeout)
           clearTimeout(timeout);
        is_open = true;
        dispatch({action: 'insert', object_name: 'local.connection'})
      }
		};

    this.open();

    this.bind = function(action, object_name, callback){
      var key = action + object_name;
      callbacks[key] = callbacks[key] || [];
      callbacks[key].push(callback);
      return this;// chainable
    };

    // XXX bad
    this.send = function(query){
      var payload = JSON.stringify(query);
      ws.send( payload ); // <= send JSON data to socket server
      return this;
    };


    var dispatch = function(query) {
      var key = query.action + query.object_name;
      var chain = callbacks[key];
      if(typeof chain === 'undefined') {
        return; // no callbacks for this event
      }
      for(var i = 0; i < chain.length; i++){
        chain[i](query)
      }
    }

  };
});
