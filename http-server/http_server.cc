/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2016 Ole Christian Eidheim
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "http_server.h"

namespace icn_httpserver {

HttpServer::HttpServer(unsigned short port,
                       std::string icn_name, size_t num_threads, long timeout_request, long timeout_send_or_receive)
    : config_(port, num_threads),
      icn_name_(icn_name),
      internal_io_service_(std::make_shared<boost::asio::io_service>()),
      io_service_(*internal_io_service_),
      acceptor_(io_service_),
      acceptor_producer_(std::make_shared<icnet::ProducerSocket>(icnet::Name(icn_name))),
      timeout_request_(timeout_request),
      timeout_content_(timeout_send_or_receive) {
}

HttpServer::HttpServer(unsigned short port,
                       std::string icn_name,
                       size_t num_threads,
                       long timeout_request,
                       long timeout_send_or_receive,
                       boost::asio::io_service &ioService)
    : config_(port, num_threads),
      icn_name_(icn_name),
      io_service_(ioService),
      acceptor_(io_service_),
      acceptor_producer_(std::make_shared<icnet::ProducerSocket>(icnet::Name(icn_name))),
      timeout_request_(timeout_request),
      timeout_content_(timeout_send_or_receive) {
}

void HttpServer::processIncomingInterest(icnet::ProducerSocket &p, const icnet::Interest &interest) {
  icnet::Name complete_name = interest.getName();

  if (complete_name.getSegmentCount() <= 2) {
    std::cerr << "Received malformed name " << complete_name << ". Ignoring it." << std::endl;
    return;
  }

  icnet::Name request_name = complete_name.get(-1).isSegment() ? complete_name.getPrefix(-1) : complete_name;

  std::unique_lock<std::mutex> lock(thread_list_mtx_);
  if (icn_producers_.size() < config_.getNum_threads()) {
    if (icn_producers_.find(request_name) == icn_producers_.end()) {
      std::cout << "Received interest name: " << request_name << std::endl;
      std::shared_ptr<icnet::ProducerSocket> p = makeProducer(request_name);
      icn_producers_[request_name] = p;
      std::cout << "Starting new thread" << std::endl;
      std::thread t([this, interest, request_name, p]() {
        processInterest(request_name, p);
      });
      t.detach();
    } else {
      icn_producers_[request_name]->onInterest(complete_name, interest);
    }
  }
}

void HttpServer::signPacket(icnet::ProducerSocket &p, icnet::ContentObject &content_object) {
  // This is not really signing the packet. Signing every packet is cpu expensive.
  icnet::KeyLocator keyLocator;
  content_object.signWithSha256(keyLocator);
}

void HttpServer::processInterest(icnet::Name request_name, std::shared_ptr<icnet::ProducerSocket> p) {
  // Create timer
  std::shared_ptr<icnet::ccnx::Portal> portal;
  p->getSocketOption(icnet::GeneralTransportOptions::PORTAL, portal);
  boost::asio::io_service &ioService = portal->getIoService();

  boost::asio::deadline_timer t(ioService, boost::posix_time::seconds(5));

  std::function<void(const boost::system::error_code e)>
      wait_callback = [&ioService](const boost::system::error_code e) {
    if (!e) {
      // Be sure to delete the timer before the io_service, otherwise we'll get some strange behavior!
      ioService.stop();
    }
  };

  t.async_wait(wait_callback);

  // Get the name of the HTTP method to compute
  std::string method = request_name.get(1).toString();
  std::transform(method.begin(), method.end(), method.begin(), ::toupper);
  std::string path;

  // This is done for getting rid of useless name components such as ccnx: or ndn:
  if (request_name.getSegmentCount() > 2) {
    std::string rawPath = request_name.getSubName(2).toString();
    std::size_t pos = rawPath.find("/");
    path = rawPath.substr(pos);
  }

  std::function<void(icnet::ProducerSocket &p, const icnet::Interest &interest)>
      interest_enter_callback = [this, &wait_callback, &t](icnet::ProducerSocket &p, const icnet::Interest &interest) {
    t.cancel();
    t.expires_from_now(boost::posix_time::seconds(5));
    t.async_wait(wait_callback);
  };

  p->setSocketOption(icnet::ProducerCallbacksOptions::INTEREST_INPUT,
                     (icnet::ProducerInterestCallback) interest_enter_callback);

  // TODO The parsing of the parameters in theURL is missing!
  if (method == GET) {
    // Build new GET request to submit to the server

    std::shared_ptr<Request> request = std::make_shared<IcnRequest>(p, request_name.toString(), path, method, "1.0");

    std::static_pointer_cast<IcnRequest>(request)->getHeader()
        .insert(std::make_pair(std::string("Host"), std::string("localhost")));

    p->attach();

    find_resource(nullptr, request);
  }

  p->serveForever();

  std::unique_lock<std::mutex> lock(thread_list_mtx_);
  icn_producers_.erase(request_name);
}

std::shared_ptr<icnet::ProducerSocket> HttpServer::makeProducer(icnet::Name request_name) {
  std::shared_ptr<icnet::ProducerSocket> producer = std::make_shared<icnet::ProducerSocket>(request_name);
  //  producer->setContextOption(FAST_SIGNING, true);
  //  producer->setContextOption(DATA_TO_SECURE, (api::ProducerDataCallback) bind(&http-server::signPacket, this, _1, _2));
  producer->setSocketOption(icnet::GeneralTransportOptions::DATA_PACKET_SIZE, PACKET_SIZE);
  producer->setSocketOption(icnet::GeneralTransportOptions::OUTPUT_BUFFER_SIZE, SEND_BUFFER_SIZE);

  return producer;
}

void HttpServer::setIcnAcceptor() {
  acceptor_producer_->setSocketOption(icnet::ProducerCallbacksOptions::INTEREST_INPUT,
                                      (icnet::ProducerInterestCallback) bind(&HttpServer::processIncomingInterest,
                                                                             this,
                                                                             std::placeholders::_1,
                                                                             std::placeholders::_2));
  acceptor_producer_->dispatch();
}

void HttpServer::spawnTcpThreads() {
  if (io_service_.stopped()) {
    io_service_.reset();
  }

  boost::asio::ip::tcp::endpoint endpoint;

  if (config_.getAddress().size() > 0) {
    endpoint =
        boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(config_.getAddress()), config_.getPort());
  } else {
    endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), config_.getPort());
  }

  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::socket_base::reuse_address(config_.isReuse_address()));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  accept();

  //If num_threads>1, start m_io_service.run() in (num_threads-1) threads for thread-pooling
  socket_threads_.clear();
  for (size_t c = 1; c < config_.getNum_threads(); c++) {
    socket_threads_.emplace_back([this]() {
      io_service_.run();
    });
  }
}

void HttpServer::start() {
  //Copy the resources to opt_resource for more efficient request processing
  opt_resource_.clear();
  for (auto &res: resource) {
    for (auto &res_method: res.second) {
      auto it = opt_resource_.end();
      for (auto opt_it = opt_resource_.begin(); opt_it != opt_resource_.end(); opt_it++) {
        if (res_method.first == opt_it->first) {
          it = opt_it;
          break;
        }
      }
      if (it == opt_resource_.end()) {
        opt_resource_.emplace_back();
        it = opt_resource_.begin() + (opt_resource_.size() - 1);
        it->first = res_method.first;
      }
      it->second.emplace_back(boost::regex(res.first), res_method.second);
    }
  }

  spawnTcpThreads();
  setIcnAcceptor();



  // Wait for the rest of the threads, if any, to finish as well
  for (auto &t: socket_threads_) {
    t.join();
  }

  //  for (auto &t : icn_threads) {
  //    t.second.get();
  //  }
}

void HttpServer::stop() {
  acceptor_.close();
  acceptor_producer_.reset();
  io_service_.stop();

  for (auto p: icn_producers_) {
    std::shared_ptr<icnet::ccnx::Portal> portalPtr;
    p.second->getSocketOption(icnet::GeneralTransportOptions::PORTAL, portalPtr);
    portalPtr->getIoService().stop();
  }

  for (auto p : icn_producers_) {
    p.second.reset();
  }

}

void HttpServer::accept() {
  //Create new socket for this connection
  //Shared_ptr is used to pass temporary objects to the asynchronous functions
  std::shared_ptr<socket_type> socket = std::make_shared<socket_type>(io_service_);

  acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &ec) {
    //Immediately start accepting a new connection
    accept();

    if (!ec) {
      boost::asio::ip::tcp::no_delay option(true);
      socket->set_option(option);
      read_request_and_content(socket);
    }
  });
}

void HttpServer::send(std::shared_ptr<Response> response, SendCallback callback) const {
  response->send(callback);
}

std::shared_ptr<boost::asio::deadline_timer> HttpServer::set_timeout_on_socket(std::shared_ptr<socket_type> socket,
                                                                               long seconds) {
  std::shared_ptr<boost::asio::deadline_timer> timer = std::make_shared<boost::asio::deadline_timer>(io_service_);
  timer->expires_from_now(boost::posix_time::seconds(seconds));
  timer->async_wait([socket](const boost::system::error_code &ec) {
    if (!ec) {
      boost::system::error_code ec;
      socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
      socket->lowest_layer().close();
    }
  });
  return timer;
}

void HttpServer::read_request_and_content(std::shared_ptr<socket_type> socket) {
  // Create new streambuf (Request::streambuf) for async_read_until()
  // shared_ptr is used to pass temporary objects to the asynchronous functions
  std::shared_ptr<Request> request = std::make_shared<SocketRequest>();
  request->read_remote_endpoint_data(*socket);

  //Set timeout on the following boost::asio::async-read or write function
  std::shared_ptr<boost::asio::deadline_timer> timer;
  if (timeout_request_ > 0) {
    timer = set_timeout_on_socket(socket, timeout_request_);
  }

  boost::asio::async_read_until(*socket,
                                request->getStreambuf(),
                                "\r\n\r\n",
                                [this, socket, request, timer](const boost::system::error_code &ec,
                                                               size_t bytes_transferred) {
                                  if (timeout_request_ > 0) {
                                    timer->cancel();
                                  }
                                  if (!ec) {
                                    //request->streambuf.size() is not necessarily the same as bytes_transferred, from Boost-docs:
                                    //"After a successful async_read_until operation, the streambuf may contain additional data beyond the delimiter"
                                    //The chosen solution is to extract lines from the stream directly when parsing the header. What is left of the
                                    //streambuf (maybe some bytes of the content) is appended to in the async_read-function below (for retrieving content).
                                    size_t num_additional_bytes = request->getStreambuf().size() - bytes_transferred;

                                    if (!parse_request(request, request->getContent())) {
                                      return;
                                    }

                                    //If content, read that as well
                                    auto it = request->getHeader().find("Content-Length");
                                    if (it != request->getHeader().end()) {
                                      //Set timeout on the following boost::asio::async-read or write function
                                      std::shared_ptr<boost::asio::deadline_timer> timer;
                                      if (timeout_content_ > 0) {
                                        timer = set_timeout_on_socket(socket, timeout_content_);
                                      }
                                      unsigned long long content_length;
                                      try {
                                        content_length = stoull(it->second);
                                      } catch (const std::exception &) {
                                        return;
                                      }
                                      if (content_length > num_additional_bytes) {
                                        boost::asio::async_read(*socket,
                                                                request->getStreambuf(),
                                                                boost::asio::transfer_exactly(
                                                                    content_length - num_additional_bytes),
                                                                [this, socket, request, timer](const boost::system::error_code &ec,
                                                                                               size_t /*bytes_transferred*/) {
                                                                  if (timeout_content_ > 0) {
                                                                    timer->cancel();
                                                                  }
                                                                  if (!ec) {
                                                                    find_resource(socket, request);
                                                                  }
                                                                });
                                      } else {

                                        if (timeout_content_ > 0) {
                                          timer->cancel();
                                        }

                                        find_resource(socket, request);
                                      }
                                    } else {
                                      find_resource(socket, request);
                                    }
                                  }
                                });
}

bool HttpServer::parse_request(std::shared_ptr<Request> request, std::istream &stream) const {
  std::string line;
  getline(stream, line);
  size_t method_end;
  if ((method_end = line.find(' ')) != std::string::npos) {
    size_t path_end;
    if ((path_end = line.find(' ', method_end + 1)) != std::string::npos) {
      request->setMethod(line.substr(0, method_end));
      request->setPath(line.substr(method_end + 1, path_end - method_end - 1));

      size_t protocol_end;
      if ((protocol_end = line.find('/', path_end + 1)) != std::string::npos) {
        if (line.substr(path_end + 1, protocol_end - path_end - 1) != "HTTP") {
          return false;
        }
        request->setHttp_version(line.substr(protocol_end + 1, line.size() - protocol_end - 2));
      } else {
        return false;
      }

      getline(stream, line);
      size_t param_end;
      while ((param_end = line.find(':')) != std::string::npos) {
        size_t value_start = param_end + 1;
        if ((value_start) < line.size()) {
          if (line[value_start] == ' ') {
            value_start++;
          }
          if (value_start < line.size()) {
            request->getHeader().insert(std::make_pair(line.substr(0, param_end),
                                                       line.substr(value_start, line.size() - value_start - 1)));
          }
        }

        getline(stream, line);
      }
    } else {
      return false;
    }
  } else {
    return false;
  }
  return true;
}

void HttpServer::find_resource(std::shared_ptr<socket_type> socket, std::shared_ptr<Request> request) {
  //Find path- and method-match, and call write_response
  for (auto &res: opt_resource_) {
    if (request->getMethod() == res.first) {
      for (auto &res_path: res.second) {
        boost::smatch sm_res;
        if (boost::regex_match(request->getPath(), sm_res, res_path.first)) {
          request->setPath_match(std::move(sm_res));
          write_response(socket, request, res_path.second);
          return;
        }
      }
    }
  }
  auto it_method = default_resource.find(request->getMethod());
  if (it_method != default_resource.end()) {
    write_response(socket, request, it_method->second);
    return;
  }

  std::cout << "resource not found" << std::endl;
}

void HttpServer::write_response(std::shared_ptr<socket_type> socket,
                                std::shared_ptr<Request> request,
                                ResourceCallback &resource_function) {
  //Set timeout on the following boost::asio::async-read or write function
  std::shared_ptr<boost::asio::deadline_timer> timer;
  if (timeout_content_ > 0 && socket) {
    timer = set_timeout_on_socket(socket, timeout_content_);
  }

  Response *resp;

  if (socket) {
    resp = new SocketResponse(socket);
  } else {
    resp = new IcnResponse(std::static_pointer_cast<IcnRequest>(request)->getProducer(),
                           std::static_pointer_cast<IcnRequest>(request)->getName(),
                           std::static_pointer_cast<IcnRequest>(request)->getPath(),
                           std::static_pointer_cast<IcnRequest>(request)->getRequest_id());
  }

  auto response = std::shared_ptr<Response>(resp, [this, request, timer, socket](Response *response_ptr) {

    auto response = std::shared_ptr<Response>(response_ptr);
    response->setIsLast(true);

    send(response, [this, response, request, timer, socket](const boost::system::error_code &ec) {
      if (!ec) {
        if (socket && timeout_content_ > 0) {
          timer->cancel();
        }

        float http_version;
        try {
          http_version = stof(request->getHttp_version());
        } catch (const std::exception &) {
          return;
        }

        auto range = request->getHeader().equal_range("Connection");
        for (auto it = range.first; it != range.second; it++) {
          if (boost::iequals(it->second, "close")) {
            return;
          }
        }
        if (http_version > 1.05) {
          read_request_and_content(std::static_pointer_cast<SocketResponse>(response)->getSocket());
        }
      }
    });
  });

  try {
    resource_function(response, request);
  } catch (const std::exception &) {
    return;
  }
}

} // end namespace icn_httpserver

