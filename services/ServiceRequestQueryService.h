#pragma once

#include "../models/ServiceRequest.h"
#include "../repositories/ServiceRequestRepository.h"

#include <optional>
#include <string>
#include <vector>

enum class ServiceRequestSearchOutcome
{
    Success,
    UnsupportedStatusFilter
};

struct ServiceRequestSearchResult
{
    ServiceRequestSearchOutcome outcome;
    std::vector<ServiceRequest> serviceRequests; // empty on UnsupportedStatusFilter or no matches
};

// T11 read-only query side of Service Requests. Coordinates listing, viewing,
// search-term normalization, blank-search behavior and status-filter
// validation. All actual querying is done by ServiceRequestRepository.
class ServiceRequestQueryService
{
public:
    explicit ServiceRequestQueryService(ServiceRequestRepository& repository);

    // All persisted requests, dateRequested DESC then id DESC.
    std::vector<ServiceRequest> listServiceRequests();

    // One request by id, or std::nullopt when it does not exist.
    std::optional<ServiceRequest> viewServiceRequest(int serviceRequestId);

    // Text search (serviceType OR description) combined with an optional
    // exact status filter. Blank text means "no text condition"; blank
    // status means "all statuses".
    ServiceRequestSearchResult searchServiceRequests(
        const std::string& searchTerm,
        const std::string& statusFilter = ""
    );

private:
    ServiceRequestRepository& repository_;

    static std::string trim(const std::string& value);
    static bool isSupportedStatus(const std::string& status);
};
