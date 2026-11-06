#pragma once
#include "../models/ServiceRequest.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include <vector>

struct DashboardSummary
{
    int residentTotal = 0;
    int residentActive = 0;
    int residentInactive = 0;
    int requestTotal = 0;
    int requestPending = 0;
    int requestInProgress = 0;
    int requestCompleted = 0;
    int requestCancelled = 0;
    int requestOpen = 0;                         // derived: pending + in progress
    std::vector<ServiceRequest> recentRequests;  // at most 5
};

// T14 read-only aggregation. Asks the repositories for counts and the most
// recent requests. It never validates, writes, or touches SQL.
class DashboardService
{
public:
    DashboardService(
        ResidentRepository& residentRepository,
        ServiceRequestRepository& serviceRequestRepository);

    DashboardSummary getSummary();

private:
    ResidentRepository& residentRepository_;
    ServiceRequestRepository& serviceRequestRepository_;
};