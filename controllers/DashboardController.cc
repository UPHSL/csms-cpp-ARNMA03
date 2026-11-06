#include "DashboardController.h"
#include "../app/AppContext.h"
#include "../models/ServiceRequest.h"
#include "../services/DashboardService.h"
#include <exception>
#include <string>

using namespace drogon;

namespace
{
HttpResponsePtr applicationNotReady()
{
    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(k500InternalServerError);
    response->setBody("Application is not ready.");
    return response;
}

// Deliberately generic: no exception text, no database details.
HttpResponsePtr dashboardUnavailable()
{
    auto response = HttpResponse::newHttpResponse();
    response->setStatusCode(k500InternalServerError);
    response->setBody("The dashboard is temporarily unavailable.");
    return response;
}
}

void DashboardController::show(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto context = AppContext::current();
    if (!context)
    {
        callback(applicationNotReady());
        return;
    }

    DashboardSummary summary;
    try
    {
        summary = context->dashboard().getSummary();
    }
    catch (const std::exception&)
    {
        callback(dashboardUnavailable());
        return;
    }

    // Every inserted scalar must be a std::string (the view uses get<std::string>).
    HttpViewData data;
    data.insert("title", std::string("CSMS Dashboard"));
    data.insert("residentTotal", std::to_string(summary.residentTotal));
    data.insert("residentActive", std::to_string(summary.residentActive));
    data.insert("residentInactive", std::to_string(summary.residentInactive));
    data.insert("requestTotal", std::to_string(summary.requestTotal));
    data.insert("requestOpen", std::to_string(summary.requestOpen));
    data.insert("requestPending", std::to_string(summary.requestPending));
    data.insert("requestInProgress", std::to_string(summary.requestInProgress));
    data.insert("requestCompleted", std::to_string(summary.requestCompleted));
    data.insert("requestCancelled", std::to_string(summary.requestCancelled));
    data.insert("recentRequests", summary.recentRequests);

    callback(HttpResponse::newHttpViewResponse("Dashboard", data));
}