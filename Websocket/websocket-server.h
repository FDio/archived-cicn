//
// Created by msardara on 28/11/16.
//

#ifndef WEBSOCKETSERVER_HPP
#define WEBSOCKETSERVER_HPP

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"

typedef websocketpp::server<websocketpp::config::asio> Server;
typedef Server::message_ptr message_ptr;

typedef std::function<void(Server *,
                           websocketpp::connection_hdl,
                           message_ptr,
                           const uint8_t*,
                           std::size_t)> HandlerFunction;


using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class WebSocketServer
{
public:
  explicit
  WebSocketServer(unsigned short port);

  void
  setHandler(const HandlerFunction &handler);

  ~WebSocketServer() {};

  void
  start();

private:
  void
  onMessage(Server* s, websocketpp::connection_hdl hdl, message_ptr msg);

  Server server;

  HandlerFunction handler;

  volatile bool m_isRunning;
};


#endif WEBSOCKETSERVER_HPP
