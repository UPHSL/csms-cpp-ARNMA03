#include <drogon/drogon.h>
#include "database/Database.h"
#include <filesystem>

int main()
{
    std::filesystem::create_directories("data");

    Database database("data/csms.db");
    database.initialize();

    drogon::app()
        .setLogLevel(trantor::Logger::kWarn)
        .addListener("127.0.0.1", 8080)
        .setThreadNum(1)
        .run();

    return 0;
}