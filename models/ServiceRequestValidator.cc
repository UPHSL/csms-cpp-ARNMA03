#include "ServiceRequestValidator.h"

#include <algorithm>
#include <cctype>

ServiceRequestValidationResult ServiceRequestValidator::validate(
    const ServiceRequest& request
) const
{
    ServiceRequestValidationResult result;

    result.idUnassignedValid = !request.getId().has_value();
    result.residentIdValid = request.getResidentId() > 0;
    result.serviceTypeValid = !isBlank(request.getServiceType());
    result.descriptionValid = !isBlank(request.getDescription());
    result.dateRequestedValid = !isBlank(request.getDateRequested());
    result.statusValid = request.getStatus() == "Pending";

    return result;
}

bool ServiceRequestValidator::isValid(
    const ServiceRequest& request
) const
{
    return validate(request).isValid();
}

bool ServiceRequestValidator::isBlank(
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
