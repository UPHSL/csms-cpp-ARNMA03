#pragma once

#include <drogon/HttpController.h>

#include <functional>
#include <string>

class ResidentController : public drogon::HttpController<ResidentController>
{
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ResidentController::list, "/residents", drogon::Get);
    ADD_METHOD_TO(ResidentController::newForm, "/residents/new", drogon::Get);
    ADD_METHOD_TO(ResidentController::create, "/residents", drogon::Post);
    ADD_METHOD_TO(ResidentController::editForm, "/residents/{id}/edit", drogon::Get);
    ADD_METHOD_TO(ResidentController::update, "/residents/{id}", drogon::Post);
        ADD_METHOD_TO(ResidentController::deactivate, "/residents/{id}/deactivate", drogon::Post);
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

    void editForm(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void update(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );

    void deactivate(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
        std::string id
    );
};