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

#include "websocket-server.h"

WebSocketServer::WebSocketServer (unsigned short port)
    : m_isRunning (false)
{
  try
    {
      // Initialize Asio
      server.init_asio ();

      // Set reuse address option
      server.set_reuse_addr (true);

      // Register our message handler
      server.set_message_handler (bind (&WebSocketServer::onMessage, this, &server, ::_1, ::_2));

      // Listen on port
      server.listen (port);

    }
  catch (websocketpp::exception const &e)
    {
      std::cout << e.what () << std::endl;
    }
  catch (...)
    {
      std::cout << "Exception occurred.." << std::endl;
    }
}

void WebSocketServer::setHandler (const HandlerFunction &handler)
{
  this->handler = handler;
}

void WebSocketServer::start ()
{
  m_isRunning = true;
  int retries = 5;

  while (m_isRunning && retries > 0)
    {

      try
        {
          // Start the server accept loop
          server.start_accept ();

          //Set interrupt callbacks

          boost::asio::io_service io_service;
          boost::asio::signal_set signals (server.get_io_service (), SIGINT, SIGQUIT);

          signals.async_wait ([this] (const boost::system::error_code &errorCode, int)
                              {
                                std::cout << "Gracefully terminating websocket server" << std::endl;
                                this->m_isRunning = false;
                                this->server.stop ();
                              });

          // Start the ASIO io_service run loop
          server.run ();

        }
      catch (websocketpp::exception const &e)
        {
          std::cout << e.what () << std::endl;
        }
      catch (...)
        {
          std::cout << "Exception occurred.." << std::endl;
        }

      retries--;
    }
}

void WebSocketServer::onMessage (Server *s, websocketpp::connection_hdl hdl, message_ptr msg)
{
  try
    {
      handler (s, hdl, msg, (uint8_t *) msg->get_payload ().c_str (), msg->get_payload ().size ());
    }
  catch (const websocketpp::lib::error_code &e)
    {

      std::cout << "Reply sent failed because: " << e << "(" << e.message () << ")" << std::endl;

    }
}