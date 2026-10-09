#include "ServiceRequestStatusService.h"

ServiceRequestStatusService::ServiceRequestStatusService(
    ServiceRequestRepository& repository
)
    : repository_(repository)
{
}

ServiceRequestStatusResult ServiceRequestStatusService::changeStatus(
    int serviceRequestId,
    const std::string& requestedStatus
)
{
    // 1. The Service Request must already exist. T10 never creates one.
    std::optional<ServiceRequest> existing = repository_.findById(serviceRequestId);

    if (!existing.has_value())
    {
        return ServiceRequestStatusResult{
            ServiceRequestStatusOutcome::NotFound,
            std::nullopt
        };
    }

    // 2. The requested target must be one of the four supported statuses.
    if (!isSupportedStatus(requestedStatus))
    {
        return ServiceRequestStatusResult{
            ServiceRequestStatusOutcome::UnsupportedStatus,
            std::nullopt
        };
    }

    // 3. The transition must be allowed from the CURRENT persisted status.
    //    Nothing has been written yet, so a rejection leaves persistence as-is.
    if (!isAllowedTransition(existing->getStatus(), requestedStatus))
    {
        return ServiceRequestStatusResult{
            ServiceRequestStatusOutcome::InvalidTransition,
            std::nullopt
        };
    }

    // 4. Persist the status only, then return the re-read persisted record.
    if (!repository_.updateStatus(serviceRequestId, requestedStatus))
    {
        // The row disappeared between the read and the update.
        return ServiceRequestStatusResult{
            ServiceRequestStatusOutcome::NotFound,
            std::nullopt
        };
    }

    return ServiceRequestStatusResult{
        ServiceRequestStatusOutcome::Success,
        repository_.findById(serviceRequestId)
    };
}

bool ServiceRequestStatusService::isSupportedStatus(const std::string& status)
{
    return status == "Pending"
        || status == "In Progress"
        || status == "Completed"
        || status == "Cancelled";
}

bool ServiceRequestStatusService::isAllowedTransition(
    const std::string& currentStatus,
    const std::string& requestedStatus
)
{
    if (currentStatus == "Pending")
    {
        return requestedStatus == "In Progress"
            || requestedStatus == "Cancelled";
    }

    if (currentStatus == "In Progress")
    {
        return requestedStatus == "Completed"
            || requestedStatus == "Cancelled";
    }

    // Completed and Cancelled are terminal. An unrecognized stored status
    // also falls through here and is treated as not transitionable.
    return false;
}
