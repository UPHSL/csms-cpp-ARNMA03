#include "HttpTestSupport.h"

#include <drogon/drogon.h>

#include <cstdio>
#include <utility>

namespace
{
HttpReply send(const drogon::HttpRequestPtr& request)
{
    auto client = drogon::HttpClient::newHttpClient(
        "http://127.0.0.1:" + std::to_string(kTestPort));

    const auto outcome = client->sendRequest(request, 5.0);

    HttpReply reply;
    if (outcome.first == drogon::ReqResult::Ok && outcome.second)
    {
        reply.status = static_cast<int>(outcome.second->statusCode());
        reply.body = std::string(outcome.second->body());
        reply.location = outcome.second->getHeader("location");
    }
    return reply;
}
}

HttpReply httpGet(
    const std::string& path,
    const std::map<std::string, std::string>& params)
{
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Get);
    request->setPath(path);
    for (const auto& entry : params)
    {
        request->setParameter(entry.first, entry.second);
    }
    return send(request);
}

HttpReply httpPostForm(
    const std::string& path,
    const std::map<std::string, std::string>& params)
{
    auto request = drogon::HttpRequest::newHttpFormPostRequest();
    request->setPath(path);
    for (const auto& entry : params)
    {
        request->setParameter(entry.first, entry.second);
    }
    return send(request);
}

TestAppFixture::TestAppFixture(const std::string& databasePath)
    : databasePath_(databasePath)
{
    std::remove(databasePath_.c_str());
    context_ = std::make_shared<AppContext>(databasePath_);
    AppContext::install(context_);
}

TestAppFixture::~TestAppFixture()
{
    AppContext::clear();
    context_.reset(); // closes the database before the file is removed
    std::remove(databasePath_.c_str());
}

AppContext& TestAppFixture::context()
{
    return *context_;
}