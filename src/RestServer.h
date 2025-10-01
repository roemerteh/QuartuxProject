#pragma once

#include "Monitor.h"
#include <thread>
#include "httplib.h"

class RestServer {
public:
    RestServer(Monitor &monitor, int port);
    ~RestServer();

    void start();
    void stop();
    void setconfig(int port);
    void restart();

private:
    Monitor &monitor_;
    int port_;
    std::thread server_thread_;
    std::atomic<bool> running_{false};
    httplib::Server svr;
};
