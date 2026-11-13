#include "Database.h"

#include <stdexcept>

Database::Database(const std::string& databasePath)
    : db_(nullptr)
{
    const int result = sqlite3_open(databasePath.c_str(), &db_);

    if (result != SQLITE_OK)
    {
        std::string errorMessage = db_
            ? sqlite3_errmsg(db_)
            : "Unknown SQLite error";

        if (db_)
        {
            sqlite3_close(db_);
            db_ = nullptr;
        }

        throw std::runtime_error(
            "Failed to open SQLite database: " + errorMessage
        );
    }
}

Database::~Database()
{
    if (db_)
    {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

sqlite3* Database::getConnection() const
{
    return db_;
}

void Database::initialize()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS residents (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            address TEXT NOT NULL,
            contact_number TEXT NOT NULL,
            email TEXT NOT NULL,
            status TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS service_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            resident_id INTEGER NOT NULL,
            service_type TEXT NOT NULL,
            description TEXT NOT NULL,
            date_requested TEXT NOT NULL,
            status TEXT NOT NULL,
            FOREIGN KEY (resident_id) REFERENCES residents (id)
        );
    )";

    char* errorMessage = nullptr;

    const int result = sqlite3_exec(
        db_,
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        std::string message = errorMessage
            ? errorMessage
            : "Unknown SQLite error";

        sqlite3_free(errorMessage);

        throw std::runtime_error(
            "Failed to initialize database: " + message
        );
    }
}
