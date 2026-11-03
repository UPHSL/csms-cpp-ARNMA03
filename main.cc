#include <drogon/drogon.h>
#include "app/AppContext.h"
#include <filesystem>
#include <memory>

int main()
{
    std::filesystem::create_directories("data");

    AppContext::install(std::make_shared<AppContext>("data/csms.db"));

    drogon::app()
        .setLogLevel(trantor::Logger::kWarn)
        .addListener("127.0.0.1", 8080)
        .setThreadNum(1)
        .run();

    AppContext::clear();
    return 0;
}