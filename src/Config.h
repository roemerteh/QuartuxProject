#pragma once
#include <string>

/**
 * @class Config
 * @brief structure Config contain configuration DB and Server variables
 * offers mechanism to load configuration from a file and reload configuration
 * 
 */
struct Config {
    std::string mysql_host;     ///< myqsl host
    std::string mysql_user;     ///< mysql user
    std::string mysql_password; ///< mysql pass
    std::string mysql_db;       ///< mysql schema
    int query_interval = 10;    ///< interval to make queries
    int rest_port = 8080;       ///< port where server listen
    int threshold = 50;         ///< limit of values

    std::string path;           ///< path where configuration file is

    /**
     * @brief loadFromFile - load configuration in format json from file received as parameter
     * @param path path of configuration file
     * @return a Config structure
     * 
     */
    static Config loadFromFile(const std::string &path);
    /**
     * @brief reLoadFromFile - reload configuration in format json from file configured before in loadFromfile()
     * it doesn't need a parameter path 
     * @return return a pair (bool, bool) where 
     * first is related if DB changes were detected and 
     * second is related if server changes were detected
     */
    std::pair<bool, bool> reLoadFromFile();
};
