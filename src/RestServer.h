#pragma once

#include "Monitor.h"
#include <thread>
#include "httplib.h"

/**
 * @class RestServer
 * @brief RestServer implement a Server REST based in httplib
 * 
 * This server expose and endpoint to check the status of Monitor module
 * can start the service loading initial configuration, restart service reloding configuration 
 * 
 */
class RestServer {
public:
    /**
     * @brief constructor - load configuration in format json from file received as parameter
     * @param monitor to get the status structure
     * @param port to configure and stand up a server listen in this port
     * 
     */
    RestServer(Monitor &monitor, int port);
    
    /**
     * @brief destructor - stop the server
     * 
     */
    ~RestServer();

    /**
     * @brief start - start the service and link a thread to a lambda function which create a server listenning in port_
     * 
     */
    void start();

    /**
     * @brief stop - stop server safely wating for thread runnig
     * 
     */
    void stop();

    /**
     * @brief setconfig - change the port where server listen
     * 
     */
    void setconfig(int port);

    /**
     * @brief restart - stop and start the server again
     * 
     */
    void restart();

private:
    Monitor &monitor_;                  ///< monitor object to get status
    int port_;                          ///< port to listen
    std::thread server_thread_;         ///< thread asscociated with server listenig
    std::atomic<bool> running_{false};  ///< flag to indicate the state of the server 
    httplib::Server svr;                ///< server from httplib
};
