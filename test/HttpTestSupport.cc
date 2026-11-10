#include "HttpTestSupport.h"

#include <drogon/drogon.h>

#include <cstdio>
#include <utility>
#include <sqlite3.h>
#include <sstream>
#include <vector>

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

std::string databaseSnapshot(TestAppFixture& fixture)
{
    sqlite3* db = nullptr;
    if (sqlite3_open_v2(fixture.databasePath().c_str(), &db,
                        SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK)
    {
        const std::string err = db ? sqlite3_errmsg(db) : "unknown error";
        if (db) sqlite3_close(db);
        return "<snapshot failed: " + err + ">";
    }

    std::vector<std::string> tables;
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db,
            "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name",
            -1, &st, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(st) == SQLITE_ROW)
        {
            tables.emplace_back(
                reinterpret_cast<const char*>(sqlite3_column_text(st, 0)));
        }
    }
    sqlite3_finalize(st);

    std::ostringstream out;
    for (const auto& table : tables)
    {
        out << "[" << table << "]\n";
        const std::string sql = "SELECT * FROM \"" + table + "\" ORDER BY rowid";
        st = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr) != SQLITE_OK)
        {
            sqlite3_finalize(st);
            out << "<unreadable>\n";
            continue;
        }
        const int cols = sqlite3_column_count(st);
        while (sqlite3_step(st) == SQLITE_ROW)
        {
            for (int c = 0; c < cols; ++c)
            {
                const auto* text = sqlite3_column_text(st, c);
                out << (text ? reinterpret_cast<const char*>(text) : "NULL") << '|';
            }
            out << '\n';
        }
        sqlite3_finalize(st);
    }
    sqlite3_close(db);
    return out.str();
}
