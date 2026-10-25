#include "ResidentQueryService.h"

#include <algorithm>
#include <cctype>

ResidentQueryService::ResidentQueryService(ResidentRepository& repository)
    : repository_(repository)
{
}

std::vector<Resident> ResidentQueryService::listResidents()
{
    return repository_.findAll();
}

std::vector<Resident> ResidentQueryService::searchResidents(
    const std::string& searchTerm
)
{
    const std::string trimmedTerm = trim(searchTerm);

    if (trimmedTerm.empty())
    {
        // Blank search (including whitespace-only) means "list all".
        return listResidents();
    }

    return repository_.searchByName(trimmedTerm);
}

std::string ResidentQueryService::trim(const std::string& value)
{
    const auto isNotSpace = [](unsigned char character)
    {
        return std::isspace(character) == 0;
    };

    auto start = std::find_if(value.begin(), value.end(), isNotSpace);
    auto end = std::find_if(value.rbegin(), value.rend(), isNotSpace).base();

    if (start >= end)
    {
        return "";
    }

    return std::string(start, end);
}