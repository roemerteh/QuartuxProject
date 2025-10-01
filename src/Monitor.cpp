
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

/**
 * @class Monitor
 * @brief class Monitor make a conection to MySql DB getting information from its constructor and launch
 * a thread linked to its method loop()
 * 
 * it can reload configution and restart the conection to the BD.
 * it use the conector c++ to connect with mysql 
 * 
 * @brief Monitor Constructor set values received as parameter a apply those changes
 * @param host - is the adresss where the DB is 
 * @param user - user to DB
 * @param db - name of the schema
 * @param interval_seconds - numer of seconds to wait to make a new query
 * @param threshold - limit for values
 */
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


/* @brief setconfig - set values received as parameter to its corresponding attributes
 * @param host is the adresss where the DB is 
 * @param user user to DB
 * @param db name of the schema
 * @param interval_seconds numer of seconds to wait to make a new query
 * @param threshold limit for values
 */
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

/* @brief restart - stop the thread associated to the loop() function 
 * apply changes and launch again the thread associated to loop()
 *  
 */
void Monitor::restart()
{
    std::cout << R"({"level":"INFO","msg":"Monitor reloading configuration"})" << std::endl;
    stop();
    applyconfig();
    start();

}

/* @brief applyconfig - apply configuration with values previously modified
 * apply changes, restart conection and set schema again
 *  
 */
void Monitor::applyconfig()
{
    // connect to MySQL
    driver = sql::mysql::get_mysql_driver_instance();
    conn.reset(driver->connect(host_, user_, password_));
    conn->setSchema(db_);
}

/* @brief Monitor destructor - stop the thread safely
 *  
 */
Monitor::~Monitor() {
    stop();
}


/* @brief start - launch thread associated to loop() fucntion
 * and change the value of flag runnig to true
 *  
 */
void Monitor::start() {
    if (running_) return;
    running_ = true;
    std::cout << R"({"level":"INFO","msg":"start Monitor"})" << std::endl;
    worker_ = std::thread(&Monitor::loop, this);
}

/* @brief stop - stop the thread associated to loop() fucntion
 * and change the value of flag runnig to false  
 *
 */
void Monitor::stop() {
    if (!running_) return;
    running_ = false;
    std::cout << R"({"level":"INFO","msg":"stop Monitor waiting for thread"})" << std::endl;
    if (worker_.joinable()) worker_.join();
}

/* @brief now_iso8601_utc - return a string with time format
 *
 */
std::string Monitor::now_iso8601_utc() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}


/* @brief loop - use a connection previously setting and make a query every interval_ time 
 * write in monitor_server.log with the content processed, update thge structur status_ with each quiery
 *
 */
void Monitor::loop() {
    
    while (running_) {
        try{
         
            stmt.reset(conn->createStatement());
            res.reset(stmt->executeQuery("SELECT id, name, value, updated_at FROM metrics"));

            int rows = 0;
            int alerts = 0;

            while (res->next()) 
            {
                ++rows;
                
                int id = res->getInt("id");
                std::string name = res->getString("name");
                int value = res->getInt("value");
                std::string updated_at = res->getString("updated_at");

                std::string msg = std::to_string(id)+" "+name+" "+std::to_string(value)+" "+updated_at;
                //std::cout << msg << std::endl;
                //Logger::GetInstance().log("INFO", msg);
                LoggerManager::Get("monitor").log("INFO", msg);

                if (value > threshold_) 
                {
                    std::string msg = "threshold exceeded";
                    //Logger::GetInstance().log("WARNING", msg);
                    LoggerManager::Get("monitor").log("WARNING", msg);
                    ++alerts;
                }
            }

            {
                std::lock_guard<std::mutex> lk(status_mtx_);
                status_.last_check = now_iso8601_utc();
                status_.rows_processed = rows;
                status_.alerts = alerts;
            }
            std::string msg = "processed "+std::to_string(rows)+" rows.";
            //Logger::GetInstance().log("INFO", msg);
            LoggerManager::Get("monitor").log("INFO", msg);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        // sleep for interval (check running_ each 1s)
        for (int i = 0; i < interval_ && running_; ++i) std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
}

/* @brief get_status - return the structure status safely using mutex 
 *
 */
MonitorStatus Monitor::get_status() {
    std::lock_guard<std::mutex> lk(status_mtx_);
    return status_;
}
