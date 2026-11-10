#pragma once

#include <string>

// Single source of truth for the four supported Service Request statuses.
// Header-only, so no CMake changes are needed.
namespace ServiceRequestStatus
{
inline bool isSupported(const std::string& status)
{
    return status == "Pending"
        || status == "In Progress"
        || status == "Completed"
        || status == "Cancelled";
}
}