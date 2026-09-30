#include "ResidentRepository.h"

#include <stdexcept>
#include <string>

namespace
{
    std::string escapeLikeWildcards(const std::string& value)
    {
        std::string escaped;
        escaped.reserve(value.size());

        for (char character : value)
        {
            if (character == '\\' || character == '%' || character == '_')
            {
                escaped.push_back('\\');
            }

            escaped.push_back(character);
        }

        return escaped;
    }
}

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
        Resident resident = mapRowToResident(statement);

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

Resident ResidentRepository::mapRowToResident(sqlite3_stmt* statement) const
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

    return Resident(
        firstName ? firstName : "",
        lastName ? lastName : "",
        address ? address : "",
        contactNumber ? contactNumber : "",
        email ? email : "",
        status ? status : "",
        id
    );
}

std::vector<Resident> ResidentRepository::findAll()
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
        ORDER BY
            last_name COLLATE NOCASE ASC,
            first_name COLLATE NOCASE ASC,
            id ASC;
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
            "Failed to prepare resident listing statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }

    std::vector<Resident> residents;

    result = sqlite3_step(statement);

    while (result == SQLITE_ROW)
    {
        residents.push_back(mapRowToResident(statement));
        result = sqlite3_step(statement);
    }

    if (result != SQLITE_DONE)
    {
        std::string errorMessage =
            sqlite3_errmsg(database_.getConnection());

        sqlite3_finalize(statement);

        throw std::runtime_error(
            "Failed to list residents: " + errorMessage
        );
    }

    sqlite3_finalize(statement);

    return residents;
}

std::vector<Resident> ResidentRepository::searchByName(
    const std::string& searchTerm
)
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
        WHERE first_name LIKE ? ESCAPE '\'
           OR last_name LIKE ? ESCAPE '\'
        ORDER BY
            last_name COLLATE NOCASE ASC,
            first_name COLLATE NOCASE ASC,
            id ASC;
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
            "Failed to prepare resident search statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }

    const std::string pattern = "%" + escapeLikeWildcards(searchTerm) + "%";

    result = sqlite3_bind_text(
        statement,
        1,
        pattern.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (result == SQLITE_OK)
    {
        result = sqlite3_bind_text(
            statement,
            2,
            pattern.c_str(),
            -1,
            SQLITE_TRANSIENT
        );
    }

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind search term.");
    }

    std::vector<Resident> residents;

    result = sqlite3_step(statement);

    while (result == SQLITE_ROW)
    {
        residents.push_back(mapRowToResident(statement));
        result = sqlite3_step(statement);
    }

    if (result != SQLITE_DONE)
    {
        std::string errorMessage =
            sqlite3_errmsg(database_.getConnection());

        sqlite3_finalize(statement);

        throw std::runtime_error(
            "Failed to search residents: " + errorMessage
        );
    }

    sqlite3_finalize(statement);

    return residents;
}

bool ResidentRepository::update(const Resident& resident)
{
    if (!resident.getId().has_value())
    {
        return false;
    }
 
    const char* sql = R"(
        UPDATE residents
        SET first_name = ?,
            last_name = ?,
            address = ?,
            contact_number = ?,
            email = ?
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
            "Failed to prepare UPDATE statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }
 
    result = sqlite3_bind_text(statement, 1, resident.getFirstName().c_str(), -1, SQLITE_TRANSIENT);
 
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 2, resident.getLastName().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 3, resident.getAddress().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 4, resident.getContactNumber().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 5, resident.getEmail().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_int(statement, 6, resident.getId().value());
 
    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind UPDATE parameters.");
    }
 
    result = sqlite3_step(statement);
 
    if (result != SQLITE_DONE)
    {
        std::string errorMessage =
            sqlite3_errmsg(database_.getConnection());
 
        sqlite3_finalize(statement);
 
        throw std::runtime_error(
            "Failed to update resident: " + errorMessage
        );
    }
 
    sqlite3_finalize(statement);
 
    return sqlite3_changes(database_.getConnection()) > 0;
}
