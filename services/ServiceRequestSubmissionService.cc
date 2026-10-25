#include "ServiceRequestSubmissionService.h"

ServiceRequestSubmissionService::ServiceRequestSubmissionService(
    ServiceRequestValidator& validator,
    ServiceRequestRepository& serviceRequestRepository,
    ResidentRepository& residentRepository
)
    : validator_(validator),
      serviceRequestRepository_(serviceRequestRepository),
      residentRepository_(residentRepository)
{
}

ServiceRequestSubmissionResult ServiceRequestSubmissionService::submit(
    const ServiceRequest& request
)
{
    // 1. Validate the Service Request's own information first.
    ServiceRequestValidationResult validationResult = validator_.validate(request);

    if (!validationResult.isValid())
    {
        return ServiceRequestSubmissionResult{
            ServiceRequestSubmissionOutcome::ValidationFailure,
            std::nullopt,
            validationResult
        };
    }

    // 2. Verify the referenced Resident exists.
    std::optional<Resident> resident = residentRepository_.findById(request.getResidentId());

    if (!resident.has_value())
    {
        return ServiceRequestSubmissionResult{
            ServiceRequestSubmissionOutcome::ResidentNotFound,
            std::nullopt,
            validationResult
        };
    }

    // 3. Verify the Resident is eligible (Active) to submit a new request.
    if (resident->getStatus() != "Active")
    {
        return ServiceRequestSubmissionResult{
            ServiceRequestSubmissionOutcome::ResidentInactive,
            std::nullopt,
            validationResult
        };
    }

    // 4. Persist only after every prior check has passed.
    ServiceRequest savedRequest = serviceRequestRepository_.save(request);

    return ServiceRequestSubmissionResult{
        ServiceRequestSubmissionOutcome::Success,
        savedRequest,
        validationResult
    };
}
