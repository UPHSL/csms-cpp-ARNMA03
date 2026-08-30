#include "ResidentRepository.h"

#include <stdexcept>
#include <string>

ResidentRepository::ResidentRepository(Database& database)
    : database_(database)
{
}

Resident ResidentRepository::save(const Resident& resident)
{
    const char* sql = R"(
        INSERT INTO residents (
            first_name,
            last_name,
            address,
            contact_number,
            email,
            status
        )
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        database_.getConnection(),
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to prepare INSERT statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }

    result = sqlite3_bind_text(
        statement,
        1,
        resident.getFirstName().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind first name.");
    }

    result = sqlite3_bind_text(
        statement,
        2,
        resident.getLastName().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind last name.");
    }

    result = sqlite3_bind_text(
        statement,
        3,
        resident.getAddress().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind address.");
    }

    result = sqlite3_bind_text(
        statement,
        4,
        resident.getContactNumber().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind contact number.");
    }

    result = sqlite3_bind_text(
        statement,
        5,
        resident.getEmail().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind email.");
    }

    result = sqlite3_bind_text(
        statement,
        6,
        resident.getStatus().c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind status.");
    }

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE)
    {
        std::string errorMessage =
            sqlite3_errmsg(database_.getConnection());

        sqlite3_finalize(statement);

        throw std::runtime_error(
            "Failed to save resident: " + errorMessage
        );
    }

    sqlite3_finalize(statement);

    const sqlite3_int64 generatedId =
        sqlite3_last_insert_rowid(database_.getConnection());

    return Resident(
        resident.getFirstName(),
        resident.getLastName(),
        resident.getAddress(),
        resident.getContactNumber(),
        resident.getEmail(),
        resident.getStatus(),
        static_cast<int>(generatedId)
    );
}

std::optional<Resident> ResidentRepository::findById(int residentId)
{
    const char* sql = R"(
        SELECT
            id,
            first_name,
            last_name,
            address,
            contact_number,
            email,
            status
        FROM residents
        WHERE id = ?;
    )";

    sqlite3_stmt* statement = nullptr;

    int result = sqlite3_prepare_v2(
        database_.getConnection(),
        sql,
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        throw std::runtime_error(
            "Failed to prepare SELECT statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }

    result = sqlite3_bind_int(statement, 1, residentId);

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind resident ID.");
    }

    result = sqlite3_step(statement);

    if (result == SQLITE_ROW)
    {
        const int id = sqlite3_column_int(statement, 0);

        const char* firstName =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 1));

        const char* lastName =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));

        const char* address =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));

        const char* contactNumber =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));

        const char* email =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));

        const char* status =
            reinterpret_cast<const char*>(sqlite3_column_text(statement, 6));

        Resident resident(
            firstName ? firstName : "",
            lastName ? lastName : "",
            address ? address : "",
            contactNumber ? contactNumber : "",
            email ? email : "",
            status ? status : "",
            id
        );

        sqlite3_finalize(statement);

        return resident;
    }

    if (result == SQLITE_DONE)
    {
        sqlite3_finalize(statement);
        return std::nullopt;
    }

    std::string errorMessage =
        sqlite3_errmsg(database_.getConnection());

    sqlite3_finalize(statement);

    throw std::runtime_error(
        "Failed to retrieve resident: " + errorMessage
    );
}