#ifndef MICROSVC_CONTROLLER
#define MICROSVC_CONTROLLER

#include "basic_controller.hpp"
#include "controller_interface.hpp"
#include "../database/data_mapper.hpp"

using namespace controller_tools;

class UParkController : public BasicController, ControllerInterface {
    public:
        UParkController() {}
        ~UParkController() {}
        void handlePost(http_request message) override;
        void handleGet(http_request message) override;
        void handlePut(http_request message) override;
        void handleDelete(http_request message) override;
        void initHttpMethodHandlers() override;

    private:
        static json::value responseNotImplemented(const http::method & method);
        static std::tuple<bool, User> userAuthentication(http_request request);
};

#endif
