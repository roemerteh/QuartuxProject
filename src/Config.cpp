#include "Config.h"
#include <fstream>
#include <iostream>
//#include "nlohmann/json.hpp"
#include "json.hpp"

using json = nlohmann::json;

Config Config::loadFromFile(const std::string &path) 
{
    Config cfg;
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    cfg.path = path;
    json j;
    ifs >> j;
    cfg.mysql_host = j.value("mysql_host", "127.0.0.1");
    cfg.mysql_user = j.value("mysql_user", "monitor");
    cfg.mysql_password = j.value("mysql_password", "");
    cfg.mysql_db = j.value("mysql_db", "monitoring");
    cfg.query_interval = j.value("query_interval", 10);
    cfg.rest_port = j.value("rest_port", 8080);
    cfg.threshold = j.value("threshold", 50);

    return cfg;
}

std::pair<bool, bool> Config::reLoadFromFile()
{
    bool changesMonitor=false;
    bool changesServer=false;
    Config cfg;
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }
    json j;
    ifs >> j;
    cfg.mysql_host = j.value("mysql_host", "127.0.0.1");
    cfg.mysql_user = j.value("mysql_user", "monitor");
    cfg.mysql_password = j.value("mysql_password", "");
    cfg.mysql_db = j.value("mysql_db", "monitoring");
    cfg.query_interval = j.value("query_interval", 10);
    cfg.rest_port = j.value("rest_port", 8080);
    cfg.threshold = j.value("threshold", 50);

    if (cfg.mysql_host!=this->mysql_host || 
        cfg.mysql_user!=this->mysql_user ||
        cfg.mysql_password!=this->mysql_password ||
        cfg.mysql_db!=this->mysql_db ||
        cfg.query_interval!=this->query_interval ||
        //cfg.rest_port!=this->rest_port ||
        cfg.threshold!=this->threshold 
        )
    {
        
        this->mysql_host = cfg.mysql_host;
        this->mysql_user = cfg.mysql_user;
        this->mysql_password = cfg.mysql_password;
        this->mysql_db = cfg.mysql_db;
        this->query_interval = cfg.query_interval;
        //this->rest_port = cfg.rest_port;
        this->threshold = cfg.threshold;
        changesMonitor = true;
    }
    if (cfg.rest_port!=this->rest_port)
    {
        this->rest_port = cfg.rest_port;
        changesServer = true;
    }

    return std::make_pair(changesMonitor, changesServer);
}
