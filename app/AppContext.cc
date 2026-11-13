#include "AppContext.h"

#include <mutex>
#include <utility>

namespace
{
std::mutex instanceMutex;
std::shared_ptr<AppContext> installedContext;
}

AppContext::AppContext(const std::string& databasePath)
    : database_(databasePath),
      residentRepository_(database_),
      serviceRequestRepository_(database_),
      residentRegistration_(residentValidator_, residentRepository_),
      residentQuery_(residentRepository_),
      residentUpdate_(residentValidator_, residentRepository_),
      residentDeactivation_(residentRepository_),
      serviceRequestSubmission_(
          serviceRequestValidator_,
          serviceRequestRepository_,
          residentRepository_),
      serviceRequestStatus_(serviceRequestRepository_),
      serviceRequestQuery_(serviceRequestRepository_),
      dashboard_(residentRepository_, serviceRequestRepository_)
{
    database_.initialize();
}

ResidentRegistrationService& AppContext::residentRegistration() { return residentRegistration_; }
ResidentQueryService& AppContext::residentQuery() { return residentQuery_; }
ResidentUpdateService& AppContext::residentUpdate() { return residentUpdate_; }
ResidentDeactivationService& AppContext::residentDeactivation() { return residentDeactivation_; }
ServiceRequestSubmissionService& AppContext::serviceRequestSubmission() { return serviceRequestSubmission_; }
ServiceRequestStatusService& AppContext::serviceRequestStatus() { return serviceRequestStatus_; }
ServiceRequestQueryService& AppContext::serviceRequestQuery() { return serviceRequestQuery_; }
DashboardService& AppContext::dashboard() { return dashboard_; }

void AppContext::install(std::shared_ptr<AppContext> context)
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    installedContext = std::move(context);
}

std::shared_ptr<AppContext> AppContext::current()
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    return installedContext;
}

void AppContext::clear()
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    installedContext.reset();
}