#pragma once

#include "../models/ServiceRequest.h"
#include "../repositories/ServiceRequestRepository.h"

#include <optional>
#include <string>

enum class ServiceRequestStatusOutcome
{
    Success,
    NotFound,
    UnsupportedStatus,
    InvalidTransition
};

struct ServiceRequestStatusResult
{
    ServiceRequestStatusOutcome outcome;
    std::optional<ServiceRequest> serviceRequest; // set only on Success
};

// Applies the T10 workflow rules, then asks the repository to persist
// only transitions that passed every check.
class ServiceRequestStatusService
{
public:
    explicit ServiceRequestStatusService(ServiceRequestRepository& repository);

    ServiceRequestStatusResult changeStatus(
        int serviceRequestId,
        const std::string& requestedStatus
    );

private:
    ServiceRequestRepository& repository_;

    static bool isSupportedStatus(const std::string& status);
    static bool isAllowedTransition(
        const std::string& currentStatus,
        const std::string& requestedStatus
    );
};
