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
#include "tcp-server.h"

TcpServer::TcpServer(unsigned short port, long read_timeout)
        : port(port)
        , acceptor(io_service)
        , read_timeout(read_timeout)
{}

TcpServer::~TcpServer()
{}

void
TcpServer::setHandler(const HandlerFunction &handler)
{
  this->handler = handler;
}

void
TcpServer::start()
{
  if (io_service.stopped())
    io_service.reset();

  asio::ip::tcp::endpoint endpoint;
  endpoint = asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port);

  acceptor.open(endpoint.protocol());
  acceptor.set_option(asio::socket_base::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.listen();

  accept();

  //Set interrupt callbacks

  asio::io_service io_service;
  asio::signal_set signals(io_service, SIGINT, SIGQUIT);

  signals.async_wait([this] (const std::error_code &errorCode, int) {
    std::cout << "Gracefully terminating tcp server" << std::endl;
    this->io_service.reset();
    this->acceptor.cancel();
  });

  io_service.run();
}

void
TcpServer::accept()
{
  //Create new socket for this connection
  //Shared_ptr is used to pass temporary objects to the asynchronous functions
  std::shared_ptr<asio::ip::tcp::socket> socket(new asio::ip::tcp::socket(io_service));
  acceptor.async_accept(*socket, [this, socket](const std::error_code &ec) {
    accept();

    if (ec) {
      if (ec == asio::error::operation_aborted) // when the socket is closed by someone
        return;
    }

    processIncomingData(socket);
  });
}

void
TcpServer::processIncomingData(std::shared_ptr<asio::ip::tcp::socket> socket)
{
  // Set timeout on the following boost::asio::async-read or write function
  std::shared_ptr<asio::steady_timer> timer;
  if (read_timeout > 0)
    timer = set_timeout_on_socket(socket, read_timeout);

  std::shared_ptr<asio::streambuf> buffer(new asio::streambuf());

  asio::async_read_until(*socket, *buffer, "\r\n\r\n",
                                [this, timer, buffer, socket](const std::error_code& error, std::size_t bytes_transferred) {
    if (read_timeout > 0)
      timer->cancel();

    if (error) {
      std::cerr << "Boost error code is not null! ERROR: " << error << std::endl;
      return;
    }

    std::size_t bufferSize = buffer->size();
    buffer->commit(buffer->size());
    const uint8_t *data = asio::buffer_cast<const uint8_t *>(buffer->data());

    std::string reply = handler(data, bufferSize);

    if (reply != "") {

      asio::async_write(*socket, asio::buffer(reply.c_str(), reply.size()), [this]
      (std::error_code ec, std::size_t /*length*/)
      {
        if (!ec) {
          std::cout << "Reply sent!" << std::endl;
        } else {
          std::cerr << "ERROR! Reply not sent." << std::endl;
        }
      });

    }
  });

}

std::shared_ptr<asio::steady_timer>
TcpServer::set_timeout_on_socket(std::shared_ptr<asio::ip::tcp::socket> socket, long seconds)
{
  std::shared_ptr<asio::steady_timer> timer(new asio::steady_timer(io_service));
  timer->expires_from_now(std::chrono::seconds(seconds));
  timer->async_wait([socket](const std::error_code &ec) {
    if (!ec) {
      std::error_code ec;
      std::cout << "Connection timeout!" << std::endl;
      socket->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
      socket->lowest_layer().close();
    }
  });
  return timer;
}
