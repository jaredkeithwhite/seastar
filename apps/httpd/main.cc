/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * Copyright 2015 Cloudius Systems
 */

#include <seastar/http/httpd.hh>
#include <seastar/http/handlers.hh>
#include <seastar/http/function_handlers.hh>
#include <seastar/http/file_handler.hh>
#include "demo.json.hh"
#include <seastar/http/api_docs.hh>

namespace bpo = boost::program_options;

using namespace seastar;
using namespace httpd;

class handl : public httpd::handler_base {
public:
    virtual future<std::unique_ptr<reply> > handle(const sstring& path,
            std::unique_ptr<request> req, std::unique_ptr<reply> rep) {
        rep->_content = "hello";
        rep->done("html");
        return make_ready_future<std::unique_ptr<reply>>(std::move(rep));
    }
};

void set_routes(routes& r) {
    function_handler* h1 = new function_handler([](const_req req) {
        return "hello";
    });
    function_handler* h2 = new function_handler([](std::unique_ptr<request> req) {
        return make_ready_future<json::json_return_type>("json-future");
    });
    r.add(operation_type::GET, url("/"), h1);
    r.add(operation_type::GET, url("/jf"), h2);
    r.add(operation_type::GET, url("/file").remainder("path"),
            new directory_handler("/"));
    demo_json::hello_world.set(r, [] (const_req req) {
        demo_json::my_object obj;
        obj.var1 = req.param.at("var1");
        obj.var2 = req.param.at("var2");
        demo_json::ns_hello_world::query_enum v = demo_json::ns_hello_world::str2query_enum(req.query_parameters.at("query_enum"));
        // This demonstrate enum conversion
        obj.enum_var = v;
        return obj;
    });
}


seastar::future<> handle_connection(seastar::connected_socket s, seastar::socket_address a) {
    auto out = s.output();
    auto in = s.input();
    return do_with(std::move(s), std::move(out), std::move(in),
                   [] (auto& s, auto& out, auto& in) {
                       return seastar::repeat([&out, &in] {
                           return in.read().then([&out] (auto buf) {
                               if (buf) {
                                   return out.write(std::move(buf)).then([&out] {
                                       return out.flush();
                                   }).then([] {
                                       return seastar::stop_iteration::no;
                                   });
                               } else {
                                   return seastar::make_ready_future<seastar::stop_iteration>(
                                           seastar::stop_iteration::yes);
                               }
                           });
                       }).then([&out] {
                           return out.close();
                       });
                   });
}


seastar::future<> service_loop() {
    seastar::listen_options lo;
    lo.reuse_address = true;
    return seastar::do_with(seastar::listen(seastar::make_ipv4_address({1234}), lo),
                            [] (auto& listener) {
                                return seastar::keep_doing([&listener] () {
                                    return listener.accept().then(
                                            [] (seastar::connected_socket s, seastar::socket_address a) {
                                                // Note we ignore, not return, the future returned by
                                                // handle_connection(), so we do not wait for one
                                                // connection to be handled before accepting the next one.
                                                handle_connection(std::move(s), std::move(a));
                                            });
                                });
                            });
}

int main(int argc, char** argv) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000), "Echo Server port");
    return app.run_deprecated(argc, argv, [&] {
        auto&& config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        return service_loop();
    });
}


int main2(int ac, char** av) {
    app_template app;
    app.add_options()("port", bpo::value<uint16_t>()->default_value(10000),
            "HTTP Server port");
    return app.run_deprecated(ac, av, [&] {
        auto&& config = app.configuration();
        uint16_t port = config["port"].as<uint16_t>();
        auto server = new http_server_control();
        auto rb = make_shared<api_registry_builder>("apps/httpd/");
        server->start().then([server] {
            return server->set_routes(set_routes);
        }).then([server, rb]{
            return server->set_routes([rb](routes& r){rb->set_api_doc(r);});
        }).then([server, rb]{
            return server->set_routes([rb](routes& r) {rb->register_function(r, "demo", "hello world application");});
        }).then([server, port] {
            return server->listen(port);
        }).then([server, port] {
            std::cout << "Seastar HTTP server listening on port " << port << " ...\n";
            engine().at_exit([server] {
                return server->stop();
            });
        });

    });
}
