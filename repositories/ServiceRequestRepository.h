#pragma once

#include "../database/Database.h"
#include "../models/ServiceRequest.h"

#include <optional>

// Service Request database access only. Does not decide whether the
// Resident exists, is Active, or whether the request's fields are valid --
// those are validator / submission-service responsibilities.
class ServiceRequestRepository
{
public:
    explicit ServiceRequestRepository(Database& database);

    ServiceRequest save(const ServiceRequest& serviceRequest);

    std::optional<ServiceRequest> findById(int serviceRequestId);

    bool updateStatus(int serviceRequestId, const std::string& status);

private:
    Database& database_;

    ServiceRequest mapRowToServiceRequest(sqlite3_stmt* statement) const;
};
