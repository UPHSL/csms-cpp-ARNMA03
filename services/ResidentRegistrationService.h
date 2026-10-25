#pragma once

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"

#include <optional>

struct ResidentRegistrationResult
{
    bool success;
    std::optional<Resident> resident;          // set only when success == true
    ResidentValidationResult validationResult; // always set; check individual *Valid fields on failure
};

class ResidentRegistrationService
{
public:
    ResidentRegistrationService(
        ResidentValidator& validator,
        ResidentRepository& repository
    );

    ResidentRegistrationResult registerResident(const Resident& resident);

private:
    ResidentValidator& validator_;
    ResidentRepository& repository_;
};