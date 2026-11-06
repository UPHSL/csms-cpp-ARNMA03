#pragma once
#include <memory>
#include <string>
#include "../database/Database.h"
#include "../models/ResidentValidator.h"
#include "../models/ServiceRequestValidator.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "../services/ResidentRegistrationService.h"
#include "../services/ResidentQueryService.h"
#include "../services/ResidentUpdateService.h"
#include "../services/ResidentDeactivationService.h"
#include "../services/ServiceRequestSubmissionService.h"
#include "../services/ServiceRequestStatusService.h"
#include "../services/ServiceRequestQueryService.h"
#include "../services/DashboardService.h"

// Owns the database connection and every repository/service the web layer
// needs. Wiring only: no business rules live here.
class AppContext
{
public:
    explicit AppContext(const std::string& databasePath);

    AppContext(const AppContext&) = delete;
    AppContext& operator=(const AppContext&) = delete;

    ResidentRegistrationService& residentRegistration();
    ResidentQueryService& residentQuery();
    ResidentUpdateService& residentUpdate();
    ResidentDeactivationService& residentDeactivation();
    ServiceRequestSubmissionService& serviceRequestSubmission();
    ServiceRequestStatusService& serviceRequestStatus();
    ServiceRequestQueryService& serviceRequestQuery();
    DashboardService& dashboard();

    // Shared instance used by controllers (Drogon constructs them itself,
    // so they cannot receive the context through a constructor).
    static void install(std::shared_ptr<AppContext> context);
    static std::shared_ptr<AppContext> current(); // nullptr if none installed
    static void clear();

private:
    // Declaration order matters: members are constructed top to bottom.
    Database database_;
    ResidentValidator residentValidator_;
    ServiceRequestValidator serviceRequestValidator_;
    ResidentRepository residentRepository_;
    ServiceRequestRepository serviceRequestRepository_;
    ResidentRegistrationService residentRegistration_;
    ResidentQueryService residentQuery_;
    ResidentUpdateService residentUpdate_;
    ResidentDeactivationService residentDeactivation_;
    ServiceRequestSubmissionService serviceRequestSubmission_;
    ServiceRequestStatusService serviceRequestStatus_;
    ServiceRequestQueryService serviceRequestQuery_;
    DashboardService dashboard_;
};