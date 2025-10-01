
#include "Monitor.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>
#include "Logger.h"
#include "json.hpp"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

Monitor::Monitor(const std::string &host,
                 const std::string &user,
                 const std::string &password,
                 const std::string &db,
                 int interval_seconds,
                 int threshold)
{
    setconfig(host, user, password, db, interval_seconds, threshold);
    applyconfig();
}

void Monitor::setconfig(const std::string &host,
                 const std::string &user,
                 const std::string &password,
                 const std::string &db,
                 int interval_seconds,
                 int threshold)
{
    host_ = "tcp://"+host+":3306";
    user_=user;
    password_=password; 
    db_=db;
    interval_=interval_seconds; 
    threshold_=threshold;
}

void Monitor::restart()
{
    std::cout << R"({"level":"INFO","msg":"Monitor reloading configuration"})" << std::endl;
    stop();
    applyconfig();
    start();

}

void Monitor::applyconfig()
{
    // connect to MySQL
    driver = sql::mysql::get_mysql_driver_instance();
    conn.reset(driver->connect(host_, user_, password_));
    conn->setSchema(db_);
}

Monitor::~Monitor() 
{
    stop();
}

void Monitor::start() 
{
    if (running_) return;
    running_ = true;
    std::cout << R"({"level":"INFO","msg":"start Monitor"})" << std::endl;
    worker_ = std::thread(&Monitor::loop, this);
}

void Monitor::stop() 
{
    if (!running_) return;
    running_ = false;
    std::cout << R"({"level":"INFO","msg":"stop Monitor waiting for thread"})" << std::endl;
    if (worker_.joinable()) worker_.join();
}

std::string Monitor::now_iso8601_utc() 
{
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};

    gmtime_r(&t, &tm);

    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

void Monitor::loop() 
{    
    // while flag running_ is true continue
    // if this flag change it means that SIGINT or SIGTERM was received   
    while (running_) {
        try{
         
            // reset smart pointer related with statement to create a new statement using conection made before
            stmt.reset(conn->createStatement());

            // reset smartpointer related with the result of the query
            res.reset(stmt->executeQuery("SELECT id, name, value, updated_at FROM metrics"));

            int rows = 0;
            int alerts = 0;

            while (res->next()) 
            {
                ++rows;
                
                // filling the variables with the content of the result of the query
                int id = res->getInt("id");
                std::string name = res->getString("name");
                int value = res->getInt("value");
                std::string updated_at = res->getString("updated_at");
                std::string msg = std::to_string(id)+" "+name+" "+std::to_string(value)+" "+updated_at;

                // writing log in Monitor logfile
                LoggerManager::Get("monitor").log("INFO", msg);

                if (value > threshold_) 
                {
                    std::string msg = "threshold exceeded";

                    // writing log in Monitor logfile with warnig about exceeded value
                    LoggerManager::Get("monitor").log("WARNING", msg);
                    
                    ++alerts;
                }
            }

            {
                // filling the status structure using a mutex to keep safety
                std::lock_guard<std::mutex> lk(status_mtx_);
                status_.last_check = now_iso8601_utc();
                status_.rows_processed = rows;
                status_.alerts = alerts;
            }

            // writing a log in Monitor logfile with resume of the query proecessed
            std::string msg = "processed "+std::to_string(rows)+" rows.";
            LoggerManager::Get("monitor").log("INFO", msg);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        // sleep for interval (check running_)
        for (int i = 0; i < interval_ && running_; ++i) std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
}

MonitorStatus Monitor::get_status() 
{
    std::lock_guard<std::mutex> lk(status_mtx_);
    return status_;
}
