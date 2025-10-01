#pragma once
#include <string>

struct Config {
    std::string mysql_host;
    std::string mysql_user;
    std::string mysql_password;
    std::string mysql_db;
    int query_interval = 10;
    int rest_port = 8080;
    int threshold = 50;

    std::string path;
    static Config loadFromFile(const std::string &path);
    std::pair<bool, bool> reLoadFromFile();
};
