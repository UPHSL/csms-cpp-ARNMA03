#include "ResidentDeactivationService.h"

ResidentDeactivationService::ResidentDeactivationService(
    ResidentRepository& repository
)
    : repository_(repository)
{
}

ResidentDeactivationResult ResidentDeactivationService::deactivate(
    int residentId
)
{
    std::optional<Resident> existing = repository_.findById(residentId);

    if (!existing.has_value())
    {
        return ResidentDeactivationResult{
            ResidentDeactivationOutcome::NotFound,
            std::nullopt
        };
    }

    if (existing->getStatus() == "Inactive")
    {
        // Idempotent: already inactive, no write, no error.
        return ResidentDeactivationResult{
            ResidentDeactivationOutcome::AlreadyInactive,
            existing
        };
    }

    repository_.deactivateById(residentId);

    return ResidentDeactivationResult{
        ResidentDeactivationOutcome::Deactivated,
        repository_.findById(residentId)
    };
}
