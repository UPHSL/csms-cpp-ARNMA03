#pragma once

#include <string>

#include "Resident.h"

struct ResidentValidationResult
{
    bool firstNameValid;
    bool lastNameValid;
    bool addressValid;
    bool contactNumberValid;
    bool emailValid;
    bool statusValid;

    bool isValid() const
    {
        return firstNameValid
            && lastNameValid
            && addressValid
            && contactNumberValid
            && emailValid
            && statusValid;
    }
};

class ResidentValidator
{
public:
    ResidentValidationResult validate(
        const Resident& resident
    ) const;

    bool isValid(
        const Resident& resident
    ) const;

private:
    bool isBlank(
        const std::string& value
    ) const;

    bool isValidContactNumber(
        const std::string& value
    ) const;

    bool isValidEmail(
        const std::string& value
    ) const;

    bool isSupportedStatus(
        ResidentStatus status
    ) const;
};
