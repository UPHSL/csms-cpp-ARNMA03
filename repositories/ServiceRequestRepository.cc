#include "ServiceRequestRepository.h"

#include <stdexcept>
#include <string>

ServiceRequestRepository::ServiceRequestRepository(Database& database)
    : database_(database)
{
}

ServiceRequest ServiceRequestRepository::save(const ServiceRequest& serviceRequest)
{
    const char* sql = R"(
        INSERT INTO service_requests (
            resident_id,
            service_type,
            description,
            date_requested,
            status
        )
        VALUES (?, ?, ?, ?, ?);
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
            "Failed to prepare Service Request INSERT statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }

    result = sqlite3_bind_int(statement, 1, serviceRequest.getResidentId());

    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 2, serviceRequest.getServiceType().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 3, serviceRequest.getDescription().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 4, serviceRequest.getDateRequested().c_str(), -1, SQLITE_TRANSIENT);
    if (result == SQLITE_OK)
        result = sqlite3_bind_text(statement, 5, serviceRequest.getStatus().c_str(), -1, SQLITE_TRANSIENT);

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind Service Request parameters.");
    }

    result = sqlite3_step(statement);

    if (result != SQLITE_DONE)
    {
        std::string errorMessage =
            sqlite3_errmsg(database_.getConnection());

        sqlite3_finalize(statement);

        throw std::runtime_error(
            "Failed to save Service Request: " + errorMessage
        );
    }

    sqlite3_finalize(statement);

    const sqlite3_int64 generatedId =
        sqlite3_last_insert_rowid(database_.getConnection());

    return ServiceRequest(
        serviceRequest.getResidentId(),
        serviceRequest.getServiceType(),
        serviceRequest.getDescription(),
        serviceRequest.getDateRequested(),
        serviceRequest.getStatus(),
        static_cast<int>(generatedId)
    );
}

std::optional<ServiceRequest> ServiceRequestRepository::findById(int serviceRequestId)
{
    const char* sql = R"(
        SELECT
            id,
            resident_id,
            service_type,
            description,
            date_requested,
            status
        FROM service_requests
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
            "Failed to prepare Service Request SELECT statement: " +
            std::string(sqlite3_errmsg(database_.getConnection()))
        );
    }

    result = sqlite3_bind_int(statement, 1, serviceRequestId);

    if (result != SQLITE_OK)
    {
        sqlite3_finalize(statement);
        throw std::runtime_error("Failed to bind Service Request ID.");
    }

    result = sqlite3_step(statement);

    if (result == SQLITE_ROW)
    {
        ServiceRequest serviceRequest = mapRowToServiceRequest(statement);
        sqlite3_finalize(statement);
        return serviceRequest;
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
        "Failed to retrieve Service Request: " + errorMessage
    );
}

ServiceRequest ServiceRequestRepository::mapRowToServiceRequest(sqlite3_stmt* statement) const
{
    const int id = sqlite3_column_int(statement, 0);
    const int residentId = sqlite3_column_int(statement, 1);

    const char* serviceType =
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 2));
    const char* description =
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 3));
    const char* dateRequested =
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 4));
    const char* status =
        reinterpret_cast<const char*>(sqlite3_column_text(statement, 5));

    return ServiceRequest(
        residentId,
        serviceType ? serviceType : "",
        description ? description : "",
        dateRequested ? dateRequested : "",
        status ? status : "",
        id
    );
}
