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

/**
 * @class MonitorStatus
 * @brief class MonitorStatus is only a structure with the status
 * 
 */
struct MonitorStatus {
    std::optional<std::string> last_check;  ///< optiona variable with string type ISO8601 UTC or nullopt
    int rows_processed = 0;                 ///< indicate row processed
    int alerts = 0;                         ///< indicate allerts processed
};

/**
 * @class Monitor
 * @brief class Monitor implement a conection to MySql DB 
 * launch a thread linked to its method loop() to make queries every interval time
 * 
 * it can reload configution and restart the conection to the BD.
 * it use the mysql-conector c++ to connect with mysql 
 * 
 */
class Monitor {
public:

    /**   
    * @brief Constructor of Monitor set values received as parameter a apply those changes
    * @param host - the adresss where the DB is 
    * @param user - user to DB
    * @param db - name of the schema
    * @param interval_seconds - numer of seconds to wait to make a new query
    * @param threshold - limit for values
    * 
    */
    Monitor(const std::string &host,
            const std::string &user,
            const std::string &password,
            const std::string &db,
            int interval_seconds,
            int threshold);

    /**
     * @brief Monitor destructor - stop the thread safely
     *  
     */

    ~Monitor();

    /**
     * @brief start - launch thread associated to loop() fucntion
     * and change the value of flag runnig to true
     *  
     */
    void start();
    /**
     * @brief stop - stop the thread associated to loop() fucntion
     * and change the value of flag runnig to false  
     *
     */
    void stop();

    /**
     * @brief restart - stop the thread associated to the loop() function 
     * apply changes and launch again the thread associated to loop()
     *  
     */

    void restart();
    /**
     * @brief setconfig - set values received as parameter to its corresponding attributes
     * @param host is the adresss where the DB is 
     * @param user user to DB
     * @param db name of the schema
     * @param interval_seconds numer of seconds to wait to make a new query
     * @param threshold limit for values
     */
    void setconfig(const std::string &host,
                 const std::string &user,
                 const std::string &password,
                 const std::string &db,
                 int interval_seconds,
                 int threshold);

    /**
     * @brief get_status - return the structure status safely using mutex 
     *
     */
    MonitorStatus get_status();

private:

    /**
     * @brief loop - use a connection previously setting and make a query every interval_ time 
     * write in /var/log/monitor_server.log with the content processed, update the "structure status_" with each query
     *
     */
    void loop();

    /**
     * @brief applyconfig - apply configuration with values previously modified
     * apply changes, restart conection and set schema again
     *  
     */
    void applyconfig();

    /**
     * @brief now_iso8601_utc - return a string with time iso8601_utc format
     *
     */
    std::string now_iso8601_utc();


private:
    std::string host_;                          ///< mysql host
    std::string user_;                          ///< mysql user
    std::string password_;                      ///< mysql pass
    std::string db_;                            ///< mysql schema of DB
    int interval_;                              ///< interval time to make a query in DB
    int threshold_;                             ///< limit for values

    std::atomic<bool> running_{false};          ///< flag to verify if service is running
    std::thread worker_;                        ///< thread associated to loop() function
    std::mutex status_mtx_;                     ///< mutex to block status structure (write/read)
    MonitorStatus status_;                      ///< status structure

    std::string str_last_check;                 ///< -
    sql::mysql::MySQL_Driver *driver;           ///< mysql driver
    std::unique_ptr<sql::Connection> conn;      ///< mysql connection
    std::unique_ptr<sql::Statement> stmt;       ///< mysql statement
    std::unique_ptr<sql::ResultSet> res;        ///< mysql resultset
};
