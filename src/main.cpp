#include <iostream>
#include <string>
#include <csignal>
#include "Config.h"
#include "Monitor.h"
#include "RestServer.h"

std::atomic<bool> g_stop{false};
std::atomic<bool> g_reload{false};

// handle_sigint - handle signal related with SIGINT and SIGTERM
// change the state of the atomic variabel g_stop to true to indicate 
// to monitor and server that need to stop
void handle_sigint(int) {
    g_stop = true;
}

// handle_sighup - handle signal related with SIGHUP
// change the state of the atomic variabel g_reload to true to indicate
// that monitor and server need to reload configuration
void handle_sighup(int) {
    g_reload = true;
}

// main function can receive a parameter indicanting where is the config file
// other way it read configuration file fron config/config.json
int main(int argc, char** argv) {
    
    std::string cfg_path = "config/config.json";
    if (argc > 1) cfg_path = argv[1];

    try {
    
        // loading structure of configuration
        auto cfg = Config::loadFromFile(cfg_path);

        // crating Monitor instance passing parameter from cfg structure
        Monitor monitor(cfg.mysql_host, cfg.mysql_user, cfg.mysql_password, 
                cfg.mysql_db, cfg.query_interval, cfg.threshold);

        // creating Server passing Monitor object and port        
        RestServer server(monitor, cfg.rest_port);

        // associating signals to corresponding handlers
        std::signal(SIGINT, handle_sigint);
        std::signal(SIGTERM, handle_sigint);
        std::signal(SIGHUP, handle_sighup);

        
        // start Monitor this will launch a thread to make queries to BD every interval of time 
        // sending logs for every processing to /var/log/monitor.log
        monitor.start();

        // start Server this will launch a thread with a server listening in port 'cfg.rest_port'
        // waiting for status petitions 
        // sending logs to /var/log/server_service.log
        server.start();

        std::cout << R"({"level":"INFO","msg":"service started"})" << std::endl;

        
        // wait until signal
        while (true) 
        {
            // this process will be wating for a signal

            if (g_stop)
            {
                // if code reach this part it means that signal SIGINT or SIGTERM was captured
                // and it will out of infinite loop 
                std::cout << R"({"level":"INFO","msg":"signal SIGINT received"})" << std::endl;
                break;
            }
            else if (g_reload)
            {
                // if code reach this part it means that signal SIGINT or SIGHUP was captured
                std::cout << R"({"level":"INFO","msg":"signal SIGHUP received"})" << std::endl;

                // reload configuration and verify configuration if exist changes related with DB or server
                std::pair<bool, bool> changes = cfg.reLoadFromFile();
                
                if (changes.first)
                {   
                    // exist Monitor changes
                    // set new configuration and restart
                    monitor.setconfig(cfg.mysql_host, cfg.mysql_user, cfg.mysql_password, 
                                        cfg.mysql_db, cfg.query_interval, cfg.threshold);
                    monitor.restart();                    
                }    

                // wait 1 second to  Monitor can retart
                std::this_thread::sleep_for(std::chrono::seconds(1));

                if (changes.second)
                {   
                    // exist Server changes
                    // set new configuration and restart
                    server.setconfig(cfg.rest_port);
                    server.restart();
                }    

                // wait 1 second to  Server can retart
                std::this_thread::sleep_for(std::chrono::seconds(1));

                // changing flag to continue working
                g_reload = false;
                continue;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << R"({"level":"INFO","msg":"shutting down"})" << std::endl;

        // stop Monitor and Server waiting for threads related with thems
        server.stop();
        monitor.stop();
        
    } 
    catch (const std::exception &ex) 
    {
        std::cerr << R"({"level":"ERROR","msg":")" << ex.what() << R"("})" << std::endl;
        return 1;
    }

    return 0;
}
