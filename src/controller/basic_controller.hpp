#ifndef BASIC_CONTROLLER
#define BASIC_CONTROLLER

#include <string>
#include <vector>
#include <cpprest/http_listener.h>
#include <cpprest/http_msg.h>
#include <pplx/pplxtasks.h>

using namespace web;
using namespace http;
using namespace http::experimental::listener;

namespace controller_tools {

    class BasicController {
    protected:
        http_listener listener;

    public:
        BasicController();
        virtual ~BasicController();

        void setBaseEndpoint(const std::string & value);
        std::string getBaseEndpoint() const;
        pplx::task<void> powerOn();
        pplx::task<void> shutdown();

        virtual void initHttpMethodHandlers() {}

        std::vector<utility::string_t> requestPath(const http_request & request);

    };
}

#endif
