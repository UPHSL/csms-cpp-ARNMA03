#include "DashboardService.h"

namespace
{
constexpr int kRecentRequestLimit = 5;
}

DashboardService::DashboardService(
    ResidentRepository& residentRepository,
    ServiceRequestRepository& serviceRequestRepository)
    : residentRepository_(residentRepository),
      serviceRequestRepository_(serviceRequestRepository)
{
}

DashboardSummary DashboardService::getSummary()
{
    DashboardSummary summary;

    summary.residentTotal = residentRepository_.countAll();
    summary.residentActive = residentRepository_.countByStatus("Active");
    summary.residentInactive = residentRepository_.countByStatus("Inactive");

    summary.requestTotal = serviceRequestRepository_.countAll();
    summary.requestPending = serviceRequestRepository_.countByStatus("Pending");
    summary.requestInProgress = serviceRequestRepository_.countByStatus("In Progress");
    summary.requestCompleted = serviceRequestRepository_.countByStatus("Completed");
    summary.requestCancelled = serviceRequestRepository_.countByStatus("Cancelled");

    // Derived only. "Open" is not a stored status.
    summary.requestOpen = summary.requestPending + summary.requestInProgress;

    summary.recentRequests = serviceRequestRepository_.findRecent(kRecentRequestLimit);
    return summary;
}