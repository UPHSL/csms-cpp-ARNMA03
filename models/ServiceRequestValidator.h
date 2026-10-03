#pragma once

#include <string>

#include "ServiceRequest.h"

struct ServiceRequestValidationResult
{
    bool idUnassignedValid;     // true if id has NOT been assigned yet (new submission)
    bool residentIdValid;       // true if residentId is a positive identifier
    bool serviceTypeValid;
    bool descriptionValid;
    bool dateRequestedValid;
    bool statusValid;           // true only if status == "Pending"

    bool isValid() const
    {
        return idUnassignedValid
            && residentIdValid
            && serviceTypeValid
            && descriptionValid
            && dateRequestedValid
            && statusValid;
    }
};

// Checks only the intrinsic ServiceRequest information. Does NOT query the
// Resident database -- Resident existence/eligibility is the submission
// service's responsibility, not the validator's.
class ServiceRequestValidator
{
public:
    ServiceRequestValidationResult validate(
        const ServiceRequest& request
    ) const;

    bool isValid(
        const ServiceRequest& request
    ) const;

private:
    bool isBlank(
        const std::string& value
    ) const;
};
