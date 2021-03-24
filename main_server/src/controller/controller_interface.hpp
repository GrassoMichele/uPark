#ifndef CONTROLLER
#define CONTROLLER

#include <cpprest/http_msg.h>

using namespace web;
using namespace http;

namespace controller_tools {

    class ControllerInterface {
      public:
        virtual void handlePost(http_request request) = 0;
        virtual void handleGet(http_request request) = 0;
        virtual void handlePut(http_request request) = 0;
        virtual void handleDelete(http_request request) = 0;
    };

}

#endif
