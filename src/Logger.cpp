#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include "Logger.h"

#include "json.hpp"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

Logger::Logger(const std::string &path)
{
    _path = path; //"/var/log/monitor_service.log";
    //_path = "./monitor_service.log";
    _file.open(_path, std::ios::out | std::ios::app);
    if (!_file.is_open())
    {
        throw std::runtime_error("coulnd't open file: " + _path);
    }
}

Logger::~Logger()
{
    if (_file.is_open())
    {
        _file.close();
    }
}
/*
Logger & Logger::GetInstance()
{
    static Logger instance;

    return (instance);
}
*/
void Logger::log(const std::string &level, const std::string &msg)
{
    std::unique_lock<std::mutex> ul(m);
    ordered_json log_json;
    log_json["timestamp"]=timestamp();
    log_json["level"]=level;
    log_json["msg"]=msg;

    _file << log_json.dump() << std::endl;
    //std::cout << "write logger: " << log_json.dump() << std::endl;
}

std::string Logger::timestamp()
{
    //std::cout << "timestamp()" << std::endl;
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    return oss.str();
}

Logger & LoggerManager::Get(const std::string &name)
{
    
    auto it = loggers.find(name);
    if (it==loggers.end())
    {
        loggers[name] = std::make_unique<Logger>(loggerFiles[name]);
    } 
            
    return *loggers[name];
}


std::map<std::string, std::unique_ptr<Logger>> LoggerManager::loggers;
std::map<std::string, std::string> LoggerManager::loggerFiles={{"monitor","/var/log/monitor_service.log"}, 
                                                              {"server","/var/log/server_service.log"}};