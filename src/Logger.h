#pragma once
#include <fstream>
#include <mutex>
#include <map>
#include <memory>


class Logger
{
    public:
    void log(const std::string &level, const std::string &msg);
    explicit Logger(const std::string &path);
    ~Logger();
    Logger(const Logger &) = delete;
    Logger & operator = (const Logger &) = delete;

    private:
    std::string timestamp();

    std::ofstream _file;
    std::string _path;
    std::mutex m;
};

class LoggerManager
{
    public:
        static Logger & Get(const std::string &name);
    private:
        static std::map<std::string, std::string> loggerFiles;
        static std::map<std::string, std::unique_ptr<Logger>> loggers;
};

