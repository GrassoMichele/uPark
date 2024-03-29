#include "basic_controller.hpp"

namespace controller_tools {
    BasicController::BasicController() {}

    void BasicController::setBaseEndpoint(const std::string & value) {
        http_listener_config server_config;
        server_config.set_ssl_context_callback(
            [&](boost::asio::ssl::context& ctx)
            {
                ctx.set_options(boost::asio::ssl::context::default_workarounds);
                ctx.use_certificate_file("../utility/upark_server.crt", boost::asio::ssl::context::pem);    // relative path respect to main.cpp
                ctx.use_private_key_file("../utility/upark_server.key", boost::asio::ssl::context::pem);
            });

        listener = http_listener(value, server_config);
    }

    std::string BasicController::getBaseEndpoint() const {
        return listener.uri().to_string();
    }

    pplx::task<void> BasicController::powerOn() {
        initHttpMethodHandlers();
        return listener.open();
    }

    pplx::task<void> BasicController::shutdown() {
        return listener.close();
    }

    std::vector<utility::string_t> BasicController::requestPath(const http_request & request) {
        utility::string_t relativePath = uri::decode(request.relative_uri().path());
        return uri::split_path(relativePath);
    }

    std::map<utility::string_t, utility::string_t> BasicController::requestQuery(const http_request & request) {
        utility::string_t relativeQuery = uri::decode(request.relative_uri().query());
        return uri::split_query(relativeQuery);
    }
}
