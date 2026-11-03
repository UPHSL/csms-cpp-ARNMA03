#pragma once

#include <map>
#include <memory>
#include <string>

#include "../app/AppContext.h"

// Must match the listener in test_main.cc.
constexpr unsigned short kTestPort = 18080;

struct HttpReply
{
    int status = 0; // 0 means the request itself failed
    std::string body;
    std::string location;
};

HttpReply httpGet(
    const std::string& path,
    const std::map<std::string, std::string>& params = {});

HttpReply httpPostForm(
    const std::string& path,
    const std::map<std::string, std::string>& params = {});

// Gives one test its own throwaway database and installs it as the shared
// AppContext the controllers use. Tests run one at a time, and each request
// finishes before the next line runs, so the single connection is never
// used from two threads at once.
class TestAppFixture
{
public:
    explicit TestAppFixture(const std::string& databasePath);
    ~TestAppFixture();

    TestAppFixture(const TestAppFixture&) = delete;
    TestAppFixture& operator=(const TestAppFixture&) = delete;

    AppContext& context();

private:
    std::string databasePath_;
    std::shared_ptr<AppContext> context_;
};