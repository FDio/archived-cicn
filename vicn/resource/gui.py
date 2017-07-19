#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2017 Cisco and/or its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import http.server
import io
import os
import socket
import socketserver
import sys
import threading

from vicn.core.resource_mgr             import ResourceManager
from vicn.helpers.resource_definition   import *

DEFAULT_GUI_ADDRESS = ''
DEFAULT_GUI_PORT    = 8000

class GUIHandler(http.server.SimpleHTTPRequestHandler):
    def send_head(self):
        if self.path == '/js/settings.js':
            return self.get_settings()
        return super().send_head()

    def get_settings(self):
        host = self.request.getsockname()[0]
        port = ResourceManager().get('websocket_port');
        r = []
        r.append("var URL='ws://{}:{}';\n".format(host, port))
        enc = sys.getfilesystemencoding()
        encoded = '\n'.join(r).encode(enc, 'surrogateescape')
        f = io.BytesIO()
        f.write(encoded)
        f.seek(0)
        self.send_response(http.server.HTTPStatus.OK)
        self.send_header("Content-type", "text/javascript;")
        self.send_header("Content-Length", str(len(encoded)))
        self.end_headers()
        return f


class GUI(Resource):
    """
    Resource: GUI

    This resource is empty on purpose. It is a temporary resource used as a
    placeholder for controlling the GUI and should be deprecated in future
    releases.
    """
    address = Attribute(String, description = 'Address on which the Webserver listens',
        default = DEFAULT_GUI_ADDRESS)
    port = Attribute(Integer, description = 'Port on which the Webserver listens',
        default = DEFAULT_GUI_PORT)
    path = Attribute(String, default='www')

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        thread = threading.Thread(target = self.run)
        thread.daemon = True
        # XXX vICN should expose internal resources for interaction
        try:
            thread.start()
        except KeyboardInterrupt:
            server.shutdown()
            sys.exit(0)

    def run(self):
        if not self.path.startswith(os.path.sep):
            # XXX we might also search in the experiment folder
            base_dir = os.path.join(os.path.dirname(__file__), os.path.pardir,
                    os.path.pardir)
            web_dir = os.path.join(base_dir, self.path)
        else:
            web_dir = self.path
        os.chdir(web_dir)
        socketserver.TCPServer.allow_reuse_address = True
        httpd = socketserver.TCPServer((self.address, self.port), GUIHandler)
        httpd.serve_forever()

    def __del__(self):
        pass
