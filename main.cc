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

#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>

#include "http-server/http_server.h"
#include "http-client/http_client.h"

typedef icn_httpserver::HttpServer HttpServer;
typedef icn_httpserver::Response Response;
typedef icn_httpserver::Request Request;

namespace std {

void default_resource_send(const HttpServer &server,
                           shared_ptr<Response> response,
                           shared_ptr<ifstream> ifs,
                           shared_ptr<vector<char>> buffer,
                           std::size_t bytes_to_read) {
  streamsize read_length;

  if ((read_length = ifs->read(&(*buffer)[0], buffer->size()).gcount()) > 0) {
    response->write(&(*buffer)[0], read_length);

    if (bytes_to_read <= static_cast<streamsize>(buffer->size())) {
      // If this is the last part of the response, send it at the pointer deletion!
      return;
    }

    std::size_t to_read = bytes_to_read - read_length;
    server.send(response, [&server, response, ifs, buffer, to_read](const boost::system::error_code &ec) {
      if (!ec) {
        default_resource_send(server, response, ifs, buffer, to_read);
      } else {
        cerr << "Connection interrupted" << endl;
      }
    });
  }
}

void afterSignal(HttpServer *webServer, const boost::system::error_code &errorCode) {
  cout << "\nGracefully terminating http-server... wait." << endl;
  webServer->stop();
}

void usage(const char *programName) {
  cerr << programName << " [-p PATH_TO_ROOT_FOOT_FOLDER] [-l WEBSERVER_PREFIX] [-x PROXY_ADDRESS]\n"
       << "Web server able to publish content and generate http responses over TCP/ICN\n" << endl;

  exit(1);
}

int main(int argc, char **argv) {
  // Parse command line arguments

  string root_folder = "/var/www/html";
  string webserver_prefix = "http://webserver";
  string proxy_address = "";

  int opt = 0;

  while ((opt = getopt(argc, argv, "p:l:hx:")) != -1) {

    switch (opt) {
      case 'p':
        root_folder = optarg;
        break;
      case 'l':
        webserver_prefix = optarg;
        break;
      case 'x':
        proxy_address = optarg;
        break;
      case 'h':
      default:
        usage(argv[0]);
        break;
    }
  }

  if (!boost::filesystem::exists(boost::filesystem::path(root_folder))) {

    // Try to create it
    try {
      if (!boost::filesystem::create_directories(boost::filesystem::path(root_folder))) {
        throw boost::filesystem::filesystem_error("", boost::system::error_code());
      }
    } catch (boost::filesystem::filesystem_error) {
      std::cerr << "The web root folder " << root_folder << " does not exist and its creation failed. Exiting.."
                << std::endl;
      return (EXIT_FAILURE);
    }
  }

  std::cout << "Using web root folder: [" << root_folder << "]" << std::endl;
  std::cout << "Using locator: [" << webserver_prefix << "]" << std::endl;

  boost::asio::io_service io_service;
  HttpServer server(8080, webserver_prefix, 50, 5, 300, io_service);

  // GET for the path /info
  // Responds with some server info
  server.resource["^/info$"]["GET"] = [](shared_ptr<Response> response, shared_ptr<Request> request) {
    stringstream content_stream;
    content_stream << "<h1>This webserver is able to reply to HTTP over TCP/ICN</h1>";
    content_stream << request->getMethod() << " " << request->getPath() << " HTTP/" << request->getHttp_version()
                   << "<br>";

    for (auto &header: request->getHeader()) {
      content_stream << header.first << ": " << header.second << "<br>";
    }

    //find length of content_stream (length received using content_stream.tellp())
    content_stream.seekp(0, ios::end);

    *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n"
              << content_stream.rdbuf();
  };

  // Default GET-example. If no other matches, this anonymous function will be called.
  // Will respond with content in the web/-directory, and its subdirectories.
  // Default file: index.html
  // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
  server.default_resource["GET"] = [&server, &root_folder, &proxy_address](shared_ptr<Response> response, shared_ptr<Request> request) {
    const auto web_root_path = boost::filesystem::canonical(root_folder);

    boost::filesystem::path path = web_root_path;
    path /= request->getPath();

    if (boost::filesystem::exists(path)) {

      path = boost::filesystem::canonical(path);

      //Check if path is within web_root_path
      if (distance(web_root_path.begin(), web_root_path.end()) <= distance(path.begin(), path.end())
          && equal(web_root_path.begin(), web_root_path.end(), path.begin())) {

        if (boost::filesystem::is_directory(path)) {
          path /= "index.html";
        } // default path

        if (boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)) {

          auto ifs = make_shared<ifstream>();
          ifs->open(path.string(), ifstream::in | ios::binary);

          if (*ifs) {
            //read and send 1 MB at a time
            streamsize buffer_size = 15 * 1024 * 1024;
            auto buffer = make_shared<vector<char> >(buffer_size);

            ifs->seekg(0, ios::end);
            auto length = ifs->tellg();
            ifs->seekg(0, ios::beg);

            response->setResponseLength(length);

            icn_httpserver::SocketRequest
                *socket_request = dynamic_cast<icn_httpserver::SocketRequest *>(request.get());

            if (socket_request) {
              *response << "HTTP/1.0 200 OK\r\nContent-Length: " << length << "\r\n\r\n";
            }

            default_resource_send(server, response, ifs, buffer, length);

            return;

          }
        }
      }
    }

    if (!proxy_address.empty()) {

      // Fetch content from remote origin
      std::stringstream ss;
      if (strncmp("http://", proxy_address.c_str(), 7) || strncmp("https://", proxy_address.c_str(), 8)) {
        ss << "http://";
      }

      ss << proxy_address;
      ss << request->getPath();

      std::cout << ss.str() << std::endl;

      HTTPClient client;

      client.download(ss.str(), *response);

//      std::cout << "+++++++++++++++++++++++++++++++++++" << reply.size() << std::endl;

//      *response << reply;

      return;
    }

    string content = "Could not open path " + request->getPath();

    *response << "HTTP/1.1 404 Not found\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;

  };

  // Let the main thread to catch SIGINT and SIGQUIT
  boost::asio::signal_set signals(io_service, SIGINT, SIGQUIT);
  signals.async_wait(bind(afterSignal, &server, placeholders::_1));

  thread server_thread([&server]() {
    //Start server
    server.start();
  });

  server_thread.join();

  return 0;
}

} // end namespace std


int main(int argc, char **argv) {
  return std::main(argc, argv);
}
