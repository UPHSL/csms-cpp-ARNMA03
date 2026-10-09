#pragma once

#include <sqlite3.h>
#include <string>

class Database
{
public:
    explicit Database(const std::string& databasePath);
    ~Database();

    sqlite3* getConnection() const;

    void initialize();

private:
    sqlite3* db_;
};