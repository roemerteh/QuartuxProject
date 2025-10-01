#pragma once
#include <string>
#include <atomic>
#include <mutex>
#include <optional>
#include <thread>

#if defined(_WIN32)
#include <mysql.h>
#else
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>
#endif

struct MonitorStatus {
    std::optional<std::string> last_check; // ISO8601 UTC or nullopt
    int rows_processed = 0;
    int alerts = 0;
};

class Monitor {
public:
    Monitor(const std::string &host,
            const std::string &user,
            const std::string &password,
            const std::string &db,
            int interval_seconds,
            int threshold);

    ~Monitor();

    void start();
    void stop();
    void restart();
    void setconfig(const std::string &host,
                 const std::string &user,
                 const std::string &password,
                 const std::string &db,
                 int interval_seconds,
                 int threshold);
    MonitorStatus get_status();

private:
    void loop();
    void applyconfig();
    std::string now_iso8601_utc();


private:
    std::string host_, user_, password_, db_;
    int interval_;
    int threshold_;

    std::atomic<bool> running_{false};
    std::thread worker_;
    std::mutex status_mtx_;
    MonitorStatus status_;

    std::string str_last_check;
    sql::mysql::MySQL_Driver *driver;
    std::unique_ptr<sql::Connection> conn;
    std::unique_ptr<sql::Statement> stmt;
    std::unique_ptr<sql::ResultSet> res;
};
