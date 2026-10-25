#pragma once

#include "../models/ServiceRequest.h"
#include "../models/ServiceRequestValidator.h"
#include "../repositories/ServiceRequestRepository.h"
#include "../repositories/ResidentRepository.h"

#include <optional>

enum class ServiceRequestSubmissionOutcome
{
    Success,
    ValidationFailure,
    ResidentNotFound,
    ResidentInactive
};

struct ServiceRequestSubmissionResult
{
    ServiceRequestSubmissionOutcome outcome;
    std::optional<ServiceRequest> serviceRequest;      // set only on Success
    ServiceRequestValidationResult validationResult;   // meaningful only on ValidationFailure
};

// Coordinates: intrinsic validation -> Resident existence -> Resident
// eligibility (Active) -> persistence. Each responsibility stays in its
// own component; this class only sequences them and stops at the first
// failure.
class ServiceRequestSubmissionService
{
public:
    ServiceRequestSubmissionService(
        ServiceRequestValidator& validator,
        ServiceRequestRepository& serviceRequestRepository,
        ResidentRepository& residentRepository
    );

    ServiceRequestSubmissionResult submit(const ServiceRequest& request);

private:
    ServiceRequestValidator& validator_;
    ServiceRequestRepository& serviceRequestRepository_;
    ResidentRepository& residentRepository_;
};
