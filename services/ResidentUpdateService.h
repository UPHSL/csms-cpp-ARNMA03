#pragma once

#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../repositories/ResidentRepository.h"

#include <optional>
#include <string>

// The only fields T06 allows to change. id and status are deliberately absent.
struct ResidentUpdateInformation
{
    std::string firstName;
    std::string lastName;
    std::string address;
    std::string contactNumber;
    std::string email;
};

struct ResidentUpdateResult
{
    bool success;                              // true only when the record was updated
    bool notFound;                             // true only when no Resident has that id
    std::optional<Resident> resident;          // set only when success == true
    ResidentValidationResult validationResult; // meaningful only when validation ran (not notFound)
};

class ResidentUpdateService
{
public:
    ResidentUpdateService(
        ResidentValidator& validator,
        ResidentRepository& repository
    );

    ResidentUpdateResult updateResident(
        int residentId,
        const ResidentUpdateInformation& information
    );

private:
    ResidentValidator& validator_;
    ResidentRepository& repository_;
};
