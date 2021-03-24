#include <iostream>

#include "termination_handling.hpp"
#include "controller/upark_controller.hpp"

using namespace web;
using namespace controller_tools;

int main(int argc, const char * argv[]) {

    setTerminationHandler();

    UParkController server;
    server.setBaseEndpoint("https://localhost:50050/apis/");

    try {
        // wait for server initialization...
        server.powerOn().wait();
        std::cout << "uPark REST server active on: " << server.getBaseEndpoint() << std::endl;

        waitForTermination();

        server.shutdown().wait();
        std::cout << "Bye!" << std::endl;
    }
    catch(std::exception & e) {
        std::cerr << "Ops! Error occured: " << e.what() << std::endl;
    }

    return 0;
}
