#pragma once

#include <drogon/HttpController.h>

#include <functional>
#include <string>

class ServiceRequestController
    : public drogon::HttpController<ServiceRequestController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ServiceRequestController::list, "/service-requests", drogon::Get);
    ADD_METHOD_TO(ServiceRequestController::newForm, "/service-requests/new", drogon::Get);
    ADD_METHOD_TO(ServiceRequestController::create, "/service-requests", drogon::Post);
    ADD_METHOD_TO(ServiceRequestController::details, "/service-requests/{id}", drogon::Get);
    ADD_METHOD_TO(ServiceRequestController::start, "/service-requests/{id}/start", drogon::Post);
    ADD_METHOD_TO(ServiceRequestController::complete, "/service-requests/{id}/complete", drogon::Post);
    ADD_METHOD_TO(ServiceRequestController::cancel, "/service-requests/{id}/cancel", drogon::Post);
    METHOD_LIST_END

    void list(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void newForm(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void create(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback
    );

    void details(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void start(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void complete(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void cancel(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );
};