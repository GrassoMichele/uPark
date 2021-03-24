#include <condition_variable>
#include <mutex>
#include <iostream>
#include <csignal>

static std::condition_variable sync_var;
static std::mutex mutex;

namespace controller_tools {

    void setTerminationHandler() {
        signal(SIGINT, [](int sig){
            sync_var.notify_one();
        });
    }

    void waitForTermination() {
        std::unique_lock<std::mutex> lock { mutex };
        sync_var.wait(lock);
        std::cout << std::endl << "Server is shutting down..." << std::endl;
    }

}
