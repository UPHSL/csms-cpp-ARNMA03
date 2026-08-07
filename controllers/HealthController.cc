#include "HealthController.h"

void HealthController::asyncHandleHttpRequest(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value result;

    result["status"] = "ok";
    result["application"] = "Community Services Management System";
    result["version"] = "0.1.0";

    auto response = HttpResponse::newHttpJsonResponse(result);

    callback(response);
}