#include <CLI/CLI.hpp>
#include <logger/logger.h>

class Config {
private:
    static inline bool MUT_PROXY_ENABLED;
    static inline string MUT_PROXY_SERVER;
    static inline int MUT_PROXY_PORT;
    static inline string MUT_PROXY_SECRET;
    Config();
public:
    static const inline bool& PROXY_ENABLED = MUT_PROXY_ENABLED;
    static const inline string& PROXY_SERVER = MUT_PROXY_SERVER;
    static const inline int& PROXY_PORT = MUT_PROXY_PORT;
    static const inline string& PROXY_SECRET = MUT_PROXY_SECRET;
    static const inline int API_ID = 15307846;
    static const inline string API_HASH = "4132481003929021d07da880c84f2a33";

    static int init(int argc, const char *argv[]) {
        CLI::App app{"A TUI telegram client", "Easygram"};

        app.add_flag("-e,--proxy-enabled", MUT_PROXY_ENABLED, "Enable the proxy");
        app.add_option("-H,--proxy-host", MUT_PROXY_SERVER, "Host of the proxy");
        app.add_option("-p,--proxy-port", MUT_PROXY_PORT, "port of the proxy");
        app.add_option("-s,--proxy-secret", MUT_PROXY_SECRET, "Secret of the proxy");

        CLI11_PARSE(app, argc, argv);
        
        return 0;
    }
};