#include "ResidentRegistrationService.h"

ResidentRegistrationService::ResidentRegistrationService(
    ResidentValidator& validator,
    ResidentRepository& repository
)
    : validator_(validator),
      repository_(repository)
{
}

ResidentRegistrationResult ResidentRegistrationService::registerResident(
    const Resident& resident
)
{
    ResidentValidationResult validationResult = validator_.validate(resident);

    if (!validationResult.isValid())
    {
        return ResidentRegistrationResult{
            false,
            std::nullopt,
            validationResult
        };
    }

    Resident savedResident = repository_.save(resident);

    return ResidentRegistrationResult{
        true,
        savedResident,
        validationResult
    };
}