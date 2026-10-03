#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../models/ServiceRequest.h"
#include "../models/ServiceRequestValidator.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "../services/ServiceRequestSubmissionService.h"

#include <cstdio>
#include <string>

// ---- T09 tests (Validate and submit Service Requests) ----

namespace
{
    int saveActiveResident(ResidentRepository& repository)
    {
        Resident saved = repository.save(
            Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));
        return saved.getId().value();
    }

    int saveInactiveResident(ResidentRepository& repository)
    {
        Resident saved = repository.save(
            Resident("Ana", "Reyes", "Barangay Uno", "09181234567", "ana@example.com", "Inactive"));
        return saved.getId().value();
    }
}

DROGON_TEST(ValidServiceRequestSubmissionSucceeds)
{
    const std::string databasePath = "test_sr_submit_valid.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::Success);
    CHECK(result.serviceRequest.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(SubmittedServiceRequestReceivesGeneratedId)
{
    const std::string databasePath = "test_sr_submit_id.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "2026-09-15");

    CHECK(!request.getId().has_value());

    auto result = service.submit(request);

    CHECK(result.serviceRequest.has_value());
    CHECK(result.serviceRequest->getId().has_value());
    CHECK(result.serviceRequest->getId().value() > 0);

    std::remove(databasePath.c_str());
}

DROGON_TEST(SubmittedServiceRequestIsPersistedAndRetrievable)
{
    const std::string databasePath = "test_sr_submit_persisted.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto result = service.submit(request);
    int serviceRequestId = result.serviceRequest->getId().value();

    auto found = serviceRequestRepository.findById(serviceRequestId);
    CHECK(found.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(SubmittedServiceRequestInformationIsPreserved)
{
    const std::string databasePath = "test_sr_submit_preserved.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Certificate Request", "Requesting certificate of residency", "2026-09-16");

    auto result = service.submit(request);
    int serviceRequestId = result.serviceRequest->getId().value();

    auto found = serviceRequestRepository.findById(serviceRequestId);

    CHECK(found.has_value());
    CHECK(found->getResidentId() == residentId);
    CHECK(found->getServiceType() == "Certificate Request");
    CHECK(found->getDescription() == "Requesting certificate of residency");
    CHECK(found->getDateRequested() == "2026-09-16");
    CHECK(found->getStatus() == "Pending");

    std::remove(databasePath.c_str());
}

DROGON_TEST(SubmittedServiceRequestStatusIsPending)
{
    const std::string databasePath = "test_sr_submit_pending.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Permit Request", "Requesting a business permit", "2026-09-17");

    auto result = service.submit(request);

    CHECK(result.serviceRequest->getStatus() == "Pending");

    std::remove(databasePath.c_str());
}

DROGON_TEST(BlankServiceTypeFailsValidation)
{
    const std::string databasePath = "test_sr_submit_blank_type.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "   ", "Employment requirement", "2026-09-15");

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::ValidationFailure);
    CHECK(result.validationResult.serviceTypeValid == false);

    std::remove(databasePath.c_str());
}

DROGON_TEST(BlankDescriptionFailsValidation)
{
    const std::string databasePath = "test_sr_submit_blank_description.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "", "2026-09-15");

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::ValidationFailure);
    CHECK(result.validationResult.descriptionValid == false);

    std::remove(databasePath.c_str());
}

DROGON_TEST(InvalidServiceRequestDoesNotReachPersistence)
{
    const std::string databasePath = "test_sr_submit_invalid_no_persist.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "", "", "2026-09-15");

    service.submit(request);

    // Fresh database -- a successful save would have produced id 1.
    CHECK(!serviceRequestRepository.findById(1).has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(NonexistentResidentPreventsSubmission)
{
    const std::string databasePath = "test_sr_submit_resident_not_found.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    ServiceRequest request(
        9999, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::ResidentNotFound);
    CHECK(!serviceRequestRepository.findById(1).has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(InactiveResidentCannotSubmitNewServiceRequest)
{
    const std::string databasePath = "test_sr_submit_inactive_resident.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveInactiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::ResidentInactive);
    CHECK(!serviceRequestRepository.findById(1).has_value());

    auto resident = residentRepository.findById(residentId);
    CHECK(resident->getStatus() == "Inactive");

    std::remove(databasePath.c_str());
}

DROGON_TEST(NonPendingInitialStatusIsRejected)
{
    const std::string databasePath = "test_sr_submit_non_pending.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "2026-09-15",
        "Completed"); // attempting to submit as already-Completed

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::ValidationFailure);
    CHECK(result.validationResult.statusValid == false);
    CHECK(!serviceRequestRepository.findById(1).has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(ServiceRequestPersistsAcrossRepositoryAccess)
{
    const std::string databasePath = "test_sr_submit_cross_repo.db";
    std::remove(databasePath.c_str());

    int serviceRequestId = 0;
    int residentId = 0;

    {
        Database database(databasePath);
        database.initialize();

        ResidentRepository residentRepository(database);
        ServiceRequestRepository serviceRequestRepository(database);
        ServiceRequestValidator validator;
        ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

        residentId = saveActiveResident(residentRepository);

        ServiceRequest request(
            residentId, "Barangay Clearance", "Employment requirement", "2026-09-15");

        auto result = service.submit(request);
        serviceRequestId = result.serviceRequest->getId().value();
    }

    {
        Database database(databasePath);
        database.initialize();

        ServiceRequestRepository serviceRequestRepository(database);

        auto found = serviceRequestRepository.findById(serviceRequestId);
        CHECK(found.has_value());
        CHECK(found->getResidentId() == residentId);
    }

    std::remove(databasePath.c_str());
}

DROGON_TEST(SubmissionDoesNotModifyTheResident)
{
    const std::string databasePath = "test_sr_submit_resident_unchanged.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "2026-09-15");

    service.submit(request);

    auto resident = residentRepository.findById(residentId);

    CHECK(resident.has_value());
    CHECK(resident->getFirstName() == "Juan");
    CHECK(resident->getLastName() == "Cruz");
    CHECK(resident->getAddress() == "Barangay Santo Tomas");
    CHECK(resident->getContactNumber() == "09171234567");
    CHECK(resident->getEmail() == "juan@example.com");
    CHECK(resident->getStatus() == "Active");

    std::remove(databasePath.c_str());
}

DROGON_TEST(BlankDateRequestedFailsValidation)
{
    const std::string databasePath = "test_sr_submit_blank_date.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository residentRepository(database);
    ServiceRequestRepository serviceRequestRepository(database);
    ServiceRequestValidator validator;
    ServiceRequestSubmissionService service(validator, serviceRequestRepository, residentRepository);

    int residentId = saveActiveResident(residentRepository);

    ServiceRequest request(
        residentId, "Barangay Clearance", "Employment requirement", "   ");

    auto result = service.submit(request);

    CHECK(result.outcome == ServiceRequestSubmissionOutcome::ValidationFailure);
    CHECK(result.validationResult.dateRequestedValid == false);
    CHECK(!serviceRequestRepository.findById(1).has_value());

    std::remove(databasePath.c_str());
}
