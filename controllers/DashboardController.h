#pragma once
#include <drogon/HttpController.h>
#include <functional>

class DashboardController : public drogon::HttpController<DashboardController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DashboardController::show, "/dashboard", drogon::Get);
    METHOD_LIST_END

    void show(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );
};