#include "HealthController.h"

#include <json/json.h>

#include "app/AppInfo.h"

void HealthController::health(
    const drogon::HttpRequestPtr&,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    // Identity and availability only. No paths, config, or database details.
    Json::Value body;
    body["application"] = AppInfo::kShortName;
    body["name"] = AppInfo::kFullName;
    body["version"] = AppInfo::kVersion;
    body["status"] = "OK";

    callback(drogon::HttpResponse::newHttpJsonResponse(body));
}