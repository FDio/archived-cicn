/*
 * Copyright (c) 2018 Cisco and/or its affiliates.
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
#include <jni.h>
#include <string>
#include <stdlib.h>
#include <http-server/http_server.h>
#include <http-client/http_client_icn.h>
#include <http-client/http_client_tcp.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <android/log.h>


static bool _isRunning = false;
icn_httpserver::HttpServer *server;

void default_resource_send(const icn_httpserver::HttpServer &server,
                           std::shared_ptr<icn_httpserver::Response> response,
                           std::shared_ptr<std::ifstream> ifs,
                           std::shared_ptr<std::vector<char>> buffer,
                           std::size_t bytes_to_read) {
    std::streamsize read_length;

    if ((read_length = ifs->read(&(*buffer)[0], buffer->size()).gcount()) > 0) {
        response->write(&(*buffer)[0], read_length);

        if (bytes_to_read <= static_cast<std::streamsize>(buffer->size())) {
            return;
        }

        std::size_t to_read = bytes_to_read - read_length;
        server.send(response,
                    [&server, response, ifs, buffer, to_read](const boost::system::error_code &ec) {
                        if (!ec) {
                            default_resource_send(server, response, ifs, buffer, to_read);
                        } else {
                            __android_log_print(ANDROID_LOG_ERROR, "HttpServer",
                                                "Connection interrupted");
                        }
                    });
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_icn_httpserver_com_supportlibrary_HttpServer_start(JNIEnv *env, jobject instance,
                                                            jstring rootFolderString,
                                                            jstring tcpListenPortString,
                                                            jstring webServerPrefixString,
                                                            jstring proxyAddressString,
                                                            jstring iCNproxyAddressString) {
    std::string root_folder = "";
    int port = 8080;
    std::string webserver_prefix = "";
    std::string proxy_address = "";
    std::string icn_proxy_address = "";
    if (rootFolderString) {
        const char *rootFolderChar = env->GetStringUTFChars(rootFolderString, JNI_FALSE);
        root_folder.append(rootFolderChar);
        env->ReleaseStringUTFChars(rootFolderString, rootFolderChar);
    }

    if (tcpListenPortString) {
        const char *tcpListenPortChar = env->GetStringUTFChars(tcpListenPortString, JNI_FALSE);
        port = atoi(tcpListenPortChar);
        env->ReleaseStringUTFChars(tcpListenPortString, tcpListenPortChar);
    }
    if (webServerPrefixString) {
        const char *webServerPrefixChar = env->GetStringUTFChars(webServerPrefixString, JNI_FALSE);
        webserver_prefix.append(webServerPrefixChar);
        env->ReleaseStringUTFChars(webServerPrefixString, webServerPrefixChar);
    }
    if (proxyAddressString) {
        const char *proxyAddressChar = env->GetStringUTFChars(proxyAddressString, JNI_FALSE);
        proxy_address.append(proxyAddressChar);
        env->ReleaseStringUTFChars(proxyAddressString, proxyAddressChar);
    }

    if (iCNproxyAddressString) {
        const char *iCNproxyAddressChar = env->GetStringUTFChars(iCNproxyAddressString, JNI_FALSE);
        icn_proxy_address.append(iCNproxyAddressChar);
        env->ReleaseStringUTFChars(iCNproxyAddressString, iCNproxyAddressChar);
    }
    boost::asio::io_service io_service;
    server = new icn_httpserver::HttpServer(port, webserver_prefix, 50, 50, 300,
                                            io_service);
    _isRunning = true;
    server->resource["^/info$"]["GET"] = [](std::shared_ptr<icn_httpserver::Response> response,
                                            std::shared_ptr<icn_httpserver::Request> request) {
        std::stringstream content_stream;
        content_stream << "<h1>This webserver is able to reply to HTTP over TCP/ICN</h1>";
        content_stream << request->getMethod() << " " << request->getPath() << " HTTP/"
                       << request->getHttp_version() << "<br>";
        for (auto &header: request->getHeader()) {
            content_stream << header.first << ": " << header.second << "<br>";
        }

        content_stream.seekp(0, std::ios::end);

        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << content_stream.tellp() << "\r\n\r\n"
                  << content_stream.rdbuf();
    };


    server->default_resource["GET"] = [&root_folder, &proxy_address, &icn_proxy_address](
            std::shared_ptr<icn_httpserver::Response> response,
            std::shared_ptr<icn_httpserver::Request>
            request) {
        const auto web_root_path = boost::filesystem::canonical(root_folder);

        boost::filesystem::path path = web_root_path;
        path /= request->getPath();

        if (path.extension().string() == ".mpd") {
            response->setResponseLifetime(std::chrono::milliseconds(3000));
        }

        icn_httpserver::SocketRequest *socket_request = dynamic_cast<icn_httpserver::SocketRequest *>(request.get());

        if (boost::filesystem::exists(path)) {
            path = boost::filesystem::canonical(path);

            if (std::distance(web_root_path.begin(), web_root_path.end()) <=
                std::distance(path.begin(), path.end()) &&
                std::equal(web_root_path.begin(), web_root_path.end(), path.begin())) {
                if (boost::filesystem::is_directory(path)) {
                    path /= "index.html";
                } // default path

                if (boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)) {
                    auto ifs = std::make_shared<std::ifstream>();
                    ifs->open(path.string(), std::ifstream::in | std::ios::binary);
                    if (*ifs) {
                        std::streamsize buffer_size = 15 * 1024 * 1024;
                        auto buffer = std::make_shared<std::vector<char> >(buffer_size);

                        ifs->seekg(0, std::ios::end);
                        auto length = ifs->tellg();
                        ifs->seekg(0, std::ios::beg);

                        response->setResponseLength(length);
                        *response << "HTTP/1.0 200 OK\r\nContent-Length: " << length << "\r\n\r\n";

                        if (path.extension().string() == ".mpd") {
                            response->setResponseLifetime(std::chrono::milliseconds(1000));
                        }
                        default_resource_send(*server, response, ifs, buffer, length);

                        return;

                    }
                }
            }
        }

        std::string proxy = "";
        HTTPClient *client = nullptr;

        if (proxy_address.empty() && !icn_proxy_address.empty()) {
            proxy = icn_proxy_address;
            client = new HTTPClientIcn(20);
        } else if (!proxy_address.empty() && icn_proxy_address.empty()) {
            proxy = proxy_address;
            client = new HTTPClientTcp;
        } else if (!proxy_address.empty() && !icn_proxy_address.empty()) {
            if (socket_request) {
                proxy = icn_proxy_address;
                client = new HTTPClientIcn(20);
            } else {
                proxy = proxy_address;
                client = new HTTPClientTcp;
            }
        }

        if (!proxy.empty()) {
            std::stringstream ss;
            if (strncmp("http://", proxy.c_str(), 7) != 0) {
                if (strncmp("https://", proxy.c_str(), 8) != 0) {
                    ss << "https://";
                } else {
                    ss << "http://";
                }
            }
            ss << proxy;
            ss << request->getPath();
            std::cout << ss.str() << std::endl;
            client->download(ss.str(), *response);

            delete client;
            if (response->size() == 0) {
                *response << "HTTP/1.1 504 Gateway Timeout\r\n\r\n";
            }

            return;
        }

        std::string content = "Could not open path " + request->getPath();

        *response << "HTTP/1.1 404 Not found\r\nContent-Length: " << content.length() << "\r\n\r\n"
                  << content;
    };

    __android_log_print(ANDROID_LOG_INFO, "HttpServer", "HttpServer Starting");
    server->start();

}

extern "C"
JNIEXPORT void JNICALL
Java_icn_httpserver_com_supportlibrary_HttpServer_stop(JNIEnv *env, jobject instance) {
    server->stop();
    _isRunning = false;
    __android_log_print(ANDROID_LOG_INFO, "HttpServer", "HttpServer Stopped");

}

extern "C"
JNIEXPORT jboolean JNICALL
Java_icn_httpserver_com_supportlibrary_HttpServer_isRunning(JNIEnv *env, jobject instance) {
    __android_log_print(ANDROID_LOG_INFO, "HttpServer", "Is Running? %s", _isRunning ? "true" : "false");
    return _isRunning;

}