#include "ResidentUpdateService.h"

ResidentUpdateService::ResidentUpdateService(
    ResidentValidator& validator,
    ResidentRepository& repository
)
    : validator_(validator),
      repository_(repository)
{
}

ResidentUpdateResult ResidentUpdateService::updateResident(
    int residentId,
    const ResidentUpdateInformation& information
)
{
    std::optional<Resident> existing = repository_.findById(residentId);

    if (!existing.has_value())
    {
        // No validation ran, so the result is all-true as a neutral placeholder;
        // callers must check notFound first.
        return ResidentUpdateResult{
            false,
            true,
            std::nullopt,
            ResidentValidationResult{true, true, true, true, true, true}
        };
    }

    // Copy the existing Resident so id and status are preserved by construction.
    Resident proposed = *existing;
    proposed.setFirstName(information.firstName);
    proposed.setLastName(information.lastName);
    proposed.setAddress(information.address);
    proposed.setContactNumber(information.contactNumber);
    proposed.setEmail(information.email);

    ResidentValidationResult validationResult = validator_.validate(proposed);

    if (!validationResult.isValid())
    {
        return ResidentUpdateResult{
            false,
            false,
            std::nullopt,
            validationResult
        };
    }

    repository_.update(proposed);

    return ResidentUpdateResult{
        true,
        false,
        repository_.findById(residentId),
        validationResult
    };
}
