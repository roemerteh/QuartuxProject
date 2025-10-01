#pragma once
#include <fstream>
#include <mutex>
#include <map>
#include <memory>

/**
 * @class Logger
 * @brief class Logger implement way to write messages in a file with format jason like this
 * timestamp, level, and message.
 * in the inicitalization receive a path-file where the messages will be written
 * it use mutex to write safely using file conector.
 * 
 */
class Logger
{
    public:
    /**   
    * @brief constructor of Logger with explicit flag to avoid implicit convertion for the compiler
    * @param path - path-filename where the logs will be written
    * 
    */
    explicit Logger(const std::string &path);

    /**   
    * @brief destructor of Logger
    * 
    */
    ~Logger();

    /**   
    * @brief log is the function which set format to the message and write to the file configured before
    * @param level - lavel of log INFO|WARNING|ERROR
    * @param msg - message to be written
    * 
    */
    void log(const std::string &level, const std::string &msg);
    
    /**   
    * @brief copy constructor is removed
    * 
    */
    Logger(const Logger &) = delete;

    /**   
    * @brief assigned operator is removed
    * 
    */
    Logger & operator = (const Logger &) = delete;

    private:
    /**
     * @brief timestamp - return a string with time iso8601_utc format
     *
     */
    std::string timestamp();

    std::ofstream _file;    ///< handler to a file
    std::string _path;      ///< path where is the file
    std::mutex m;           ///< mutex to protect file of writers
};

/**
 * @class LoggerManager
 * @brief class LoggerManager provide a interface to create and get a Logger instance for every kind of log
 * for example for monitor log can have service_monitor.log and for server log can have server_service.log
 * when this class receive a reques for Get() it check if the Logger has been created if so, return the Logger associated
 * if not create a new Logger, add to the map and then return the logger.
 * 
 */
class LoggerManager
{
    public:
       /**
         * @brief Get - check if the name of the logger has been created and return it if not create a new one with that name and return it
         * @param name associated with the logger
         * @return the logger associated
         */
        static Logger & Get(const std::string &name);
    private:

        static std::map<std::string, std::string> loggerFiles;          ///< map with the names and paths-files assiciated
        static std::map<std::string, std::unique_ptr<Logger>> loggers;  ///< map with the name of loggers and pointers created
};

