#include "ServiceRequestQueryService.h"

#include <algorithm>
#include <cctype>

ServiceRequestQueryService::ServiceRequestQueryService(
    ServiceRequestRepository& repository
)
    : repository_(repository)
{
}

std::vector<ServiceRequest> ServiceRequestQueryService::listServiceRequests()
{
    return repository_.findAll();
}

std::optional<ServiceRequest> ServiceRequestQueryService::viewServiceRequest(
    int serviceRequestId
)
{
    // Reuses the T09 findById(). Read-only: a missing id returns nullopt and
    // nothing is created.
    return repository_.findById(serviceRequestId);
}

ServiceRequestSearchResult ServiceRequestQueryService::searchServiceRequests(
    const std::string& searchTerm,
    const std::string& statusFilter
)
{
    const std::string trimmedTerm = trim(searchTerm);
    const std::string trimmedStatus = trim(statusFilter);

    // A non-empty status filter must be one of the four real statuses.
    if (!trimmedStatus.empty() && !isSupportedStatus(trimmedStatus))
    {
        return ServiceRequestSearchResult{
            ServiceRequestSearchOutcome::UnsupportedStatusFilter,
            {}
        };
    }

    // Blank search with no status filter is normal listing.
    if (trimmedTerm.empty() && trimmedStatus.empty())
    {
        return ServiceRequestSearchResult{
            ServiceRequestSearchOutcome::Success,
            repository_.findAll()
        };
    }

    return ServiceRequestSearchResult{
        ServiceRequestSearchOutcome::Success,
        repository_.search(trimmedTerm, trimmedStatus)
    };
}

std::string ServiceRequestQueryService::trim(const std::string& value)
{
    const auto isNotSpace = [](unsigned char character)
    {
        return std::isspace(character) == 0;
    };

    auto start = std::find_if(value.begin(), value.end(), isNotSpace);
    auto end = std::find_if(value.rbegin(), value.rend(), isNotSpace).base();

    if (start >= end)
    {
        return "";
    }

    return std::string(start, end);
}

bool ServiceRequestQueryService::isSupportedStatus(const std::string& status)
{
    return status == "Pending"
        || status == "In Progress"
        || status == "Completed"
        || status == "Cancelled";
}
