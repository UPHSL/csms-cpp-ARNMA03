#include "ResidentValidator.h"

#include <algorithm>
#include <cctype>

ResidentValidationResult ResidentValidator::validate(
    const Resident& resident
) const
{
    ResidentValidationResult result;

    result.firstNameValid = !isBlank(resident.getFirstName());
    result.lastNameValid = !isBlank(resident.getLastName());
    result.addressValid = !isBlank(resident.getAddress());
    result.contactNumberValid = isValidContactNumber(resident.getContactNumber());
    result.emailValid = isValidEmail(resident.getEmail());
    result.statusValid = isSupportedStatus(resident.getStatus());

    return result;
}

bool ResidentValidator::isValid(
    const Resident& resident
) const
{
    return validate(resident).isValid();
}

bool ResidentValidator::isBlank(
    const std::string& value
) const
{
    if (value.empty())
    {
        return true;
    }

    return std::all_of(
        value.begin(),
        value.end(),
        [](unsigned char character)
        {
            return std::isspace(character) != 0;
        }
    );
}

bool ResidentValidator::isValidContactNumber(
    const std::string& value
) const
{
    if (value.size() != 11)
    {
        return false;
    }

    if (value[0] != '0' || value[1] != '9')
    {
        return false;
    }

    return std::all_of(
        value.begin(),
        value.end(),
        [](unsigned char character)
        {
            return std::isdigit(character) != 0;
        }
    );
}

bool ResidentValidator::isValidEmail(
    const std::string& value
) const
{
    if (value.empty())
    {
        return false;
    }

    const bool containsWhitespace =
        std::any_of(
            value.begin(),
            value.end(),
            [](unsigned char character)
            {
                return std::isspace(character) != 0;
            }
        );

    if (containsWhitespace)
    {
        return false;
    }

    const std::size_t atPosition = value.find('@');

    if (atPosition == std::string::npos
        || atPosition == 0
        || atPosition == value.size() - 1)
    {
        return false;
    }

    if (atPosition != value.rfind('@'))
    {
        return false;
    }

    const std::string domain = value.substr(atPosition + 1);

    const std::size_t dotPosition = domain.find('.');

    if (dotPosition == std::string::npos
        || dotPosition == 0
        || dotPosition == domain.size() - 1)
    {
        return false;
    }

    return true;
}

bool ResidentValidator::isSupportedStatus(
    const std::string& status
) const
{
    return status == "Active"
        || status == "Inactive";
}
