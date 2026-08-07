#include <drogon/drogon.h>

int main()
{
    drogon::app()
        .setLogLevel(trantor::Logger::kWarn)
        .addListener("127.0.0.1", 8080)
        .setThreadNum(1)
        .run();

    return 0;
}