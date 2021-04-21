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
        // waiting for server initialization...
        server.powerOn().wait();

        std::cout << R"(-------------------------------------------------------)" << std::endl;
        std::cout << R"( ___  ___  ________  ________  ________  ___  __       )" << std::endl;
        std::cout << R"(|\  \|\  \|\   __  \|\   __  \|\   __  \|\  \|\  \     )" << std::endl;
        std::cout << R"(\ \  \\\  \ \  \|\  \ \  \|\  \ \  \|\  \ \  \/  /|_   )" << std::endl;
        std::cout << R"( \ \  \\\  \ \   ____\ \   __  \ \   _  _\ \   ___  \  )" << std::endl;
        std::cout << R"(  \ \  \\\  \ \  \___|\ \  \ \  \ \  \\  \\ \  \\ \  \ )" << std::endl;
        std::cout << R"(   \ \_______\ \__\    \ \__\ \__\ \__\\ _\\ \__\\ \__\)" << std::endl;
        std::cout << R"(    \|_______|\|__|     \|__|\|__|\|__|\|__|\|__| \|__|)" << std::endl;
        std::cout << R"(                                                       )" << std::endl;
        std::cout << R"(-------------------------------------------------------)" << std::endl;

        std::cout << "uPark REST server active on: " << server.getBaseEndpoint() << std::endl;

        waitForTermination();

        // waiting for server termination...
        server.shutdown().wait();
        std::cout << "Bye!" << std::endl;        
    }
    catch(std::exception & e) {
        std::cerr << "Ops! Error occured: " << e.what() << std::endl;
    }

    return 0;
}
