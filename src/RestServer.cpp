#include <iostream>
#include "RestServer.h"
#include "json.hpp"
#include "Logger.h"

using json = nlohmann::json;
using namespace httplib;


RestServer::RestServer(Monitor &monitor, int port)
    : monitor_(monitor), port_(port) {}

RestServer::~RestServer() {
    stop();
}

void RestServer::setconfig(int port)
{
    port_= port;
}

void RestServer::restart()
{
    LoggerManager::Get("server").log("INFO", "reload configuration Server");
    stop();
    start();
}

void RestServer::start() 
{
    if (running_) return;

    LoggerManager::Get("server").log("INFO", "start Server");
    running_ = true;
    server_thread_ = std::thread([this]() {
        //Server svr;

        svr.Get("/status", [this](const Request& req, Response& res) {
            LoggerManager::Get("server").log("INFO", "status request received");
            auto st = monitor_.get_status();
            json j;
            if (st.last_check.has_value()) {
                j["last_check"] = st.last_check.value();
            } else {
                j["last_check"] = nullptr;
            }
            j["rows_processed"] = st.rows_processed;
            j["alerts"] = st.alerts;

            res.set_header("Content-Type", "application/json");
            res.status = 200;
            res.body = j.dump(4);
        });


        svr.set_exception_handler([](const Request&, Response& res, std::exception_ptr ep) {
            res.status = 500;
            try {
                if (ep) std::rethrow_exception(ep);
            } catch (const std::exception &e) {
                res.set_content(std::string("Server error: ") + e.what(), "text/plain");
            } catch (...) {
                res.set_content("Unknown server error", "text/plain");
            }
        });

        svr.listen("0.0.0.0", port_);
    });
}

void RestServer::stop() {
    if (!running_) return;
    LoggerManager::Get("server").log("INFO", "stop Server waiting for thread");
    running_ = false;
    svr.stop();
    if (server_thread_.joinable()) server_thread_.join();
}
