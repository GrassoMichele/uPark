#ifndef MICROSVC_CONTROLLER
#define MICROSVC_CONTROLLER

#include "basic_controller.hpp"
#include "controller_interface.hpp"
#include "../database/data_mapper.hpp"

#include <functional>

using namespace controller_tools;

class UParkController : public BasicController, ControllerInterface {
    public:
        UParkController() {}
        void handlePost(http_request message) override;
        void handleGet(http_request message) override;
        void handlePut(http_request message) override;
        void handleDelete(http_request message) override;
        void initHttpMethodHandlers() override;

    private:
        static std::tuple<bool, User> userAuthentication(http_request);
        static void handler_json(const http_request&, std::function<void(const http_request&, const json::value&, const User&)> handler, const User& authenticated_user);
        static void handler_auth(const http_request&, bool json_body, std::function<void(const http_request&, const json::value&, const User&)> handler);
};

#endif
