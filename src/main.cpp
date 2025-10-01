#include <iostream>
#include <string>
#include <csignal>
#include "Config.h"
#include "Monitor.h"
#include "RestServer.h"
//#include "mysql/jdbc.h"

std::atomic<bool> g_stop{false};
std::atomic<bool> g_reload{false};

void handle_sigint(int) {
    g_stop = true;
}

void handle_sighup(int) {
    g_reload = true;
}

int main(int argc, char** argv) {
    
    std::string cfg_path = "config/config.json";
    if (argc > 1) cfg_path = argv[1];

    try {
    
        auto cfg = Config::loadFromFile(cfg_path);
        Monitor monitor(cfg.mysql_host, cfg.mysql_user, cfg.mysql_password, 
                cfg.mysql_db, cfg.query_interval, cfg.threshold);
        RestServer server(monitor, cfg.rest_port);

        std::signal(SIGINT, handle_sigint);
        std::signal(SIGTERM, handle_sigint);
        std::signal(SIGHUP, handle_sighup);

        monitor.start();
        server.start();

        std::cout << R"({"level":"INFO","msg":"service started"})" << std::endl;

        
        // wait until signal
        while (true) {

            if (g_stop)
            {
                std::cout << R"({"level":"INFO","msg":"signal SIGINT received"})" << std::endl;
                break;
            }
            else if (g_reload)
            {
                std::cout << R"({"level":"INFO","msg":"signal SIGHUP received"})" << std::endl;
                std::pair<bool, bool> changes = cfg.reLoadFromFile();
                
                if (changes.first)
                {   // Monitor changes
                    monitor.setconfig(cfg.mysql_host, cfg.mysql_user, cfg.mysql_password, 
                                        cfg.mysql_db, cfg.query_interval, cfg.threshold);
                    monitor.restart();                    
                }    

                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (changes.second)
                {   // Server changes
                    server.setconfig(cfg.rest_port);
                    server.restart();
                }    

                g_reload = false;
                continue;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << R"({"level":"INFO","msg":"shutting down"})" << std::endl;
        server.stop();
        monitor.stop();
        
    } catch (const std::exception &ex) {
        std::cerr << R"({"level":"ERROR","msg":")" << ex.what() << R"("})" << std::endl;
        return 1;
    }

    return 0;
}



/*
int main()
{
    cout << "hola mundo cruel" << endl;
    
    return 0;
}
*/