#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../models/ServiceRequest.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "../services/ServiceRequestStatusService.h"
#include "../services/ResidentDeactivationService.h"

#include <cstdio>
#include <string>

// ---- T10 tests (Manage Service Request Status) ----

namespace
{
    // Owns a fresh database plus everything a T10 test needs.
    struct StatusFixture
    {
        Database database;
        ResidentRepository residentRepository;
        ServiceRequestRepository serviceRequestRepository;
        ServiceRequestStatusService service;

        explicit StatusFixture(const std::string& databasePath)
            : database(freshPath(databasePath)),
              residentRepository(database),
              serviceRequestRepository(database),
              service(serviceRequestRepository)
        {
            database.initialize();
        }

        static const std::string& freshPath(const std::string& path)
        {
            std::remove(path.c_str());
            return path;
        }
    };

    // Persists an Active Resident and a Pending Service Request for it.
    int addPendingRequest(StatusFixture& fixture)
    {
        Resident resident = fixture.residentRepository.save(
            Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));

        ServiceRequest saved = fixture.serviceRequestRepository.save(
            ServiceRequest(
                resident.getId().value(),
                "Barangay Clearance",
                "Employment requirement",
                "2026-09-15"));

        return saved.getId().value();
    }

    std::string persistedStatus(StatusFixture& fixture, int id)
    {
        return fixture.serviceRequestRepository.findById(id)->getStatus();
    }
}

// Test 1
DROGON_TEST(PendingServiceRequestCanMoveToInProgress)
{
    const std::string databasePath = "test_status_pending_to_in_progress.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    auto result = fixture.service.changeStatus(id, "In Progress");

    CHECK(result.outcome == ServiceRequestStatusOutcome::Success);
    CHECK(result.serviceRequest.has_value());
    CHECK(result.serviceRequest->getStatus() == "In Progress");
    CHECK(persistedStatus(fixture, id) == "In Progress");

    std::remove(databasePath.c_str());
}

// Test 2
DROGON_TEST(PendingServiceRequestCanMoveToCancelled)
{
    const std::string databasePath = "test_status_pending_to_cancelled.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    auto result = fixture.service.changeStatus(id, "Cancelled");

    CHECK(result.outcome == ServiceRequestStatusOutcome::Success);
    CHECK(persistedStatus(fixture, id) == "Cancelled");

    std::remove(databasePath.c_str());
}

// Test 3
DROGON_TEST(InProgressServiceRequestCanMoveToCompleted)
{
    const std::string databasePath = "test_status_in_progress_to_completed.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    // Reach In Progress through the real workflow.
    CHECK(fixture.service.changeStatus(id, "In Progress").outcome
          == ServiceRequestStatusOutcome::Success);

    auto result = fixture.service.changeStatus(id, "Completed");

    CHECK(result.outcome == ServiceRequestStatusOutcome::Success);
    CHECK(persistedStatus(fixture, id) == "Completed");

    std::remove(databasePath.c_str());
}

// Test 4
DROGON_TEST(InProgressServiceRequestCanMoveToCancelled)
{
    const std::string databasePath = "test_status_in_progress_to_cancelled.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    fixture.service.changeStatus(id, "In Progress");

    auto result = fixture.service.changeStatus(id, "Cancelled");

    CHECK(result.outcome == ServiceRequestStatusOutcome::Success);
    CHECK(persistedStatus(fixture, id) == "Cancelled");

    std::remove(databasePath.c_str());
}

// Test 5
DROGON_TEST(PendingServiceRequestCannotMoveDirectlyToCompleted)
{
    const std::string databasePath = "test_status_pending_to_completed.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    auto result = fixture.service.changeStatus(id, "Completed");

    CHECK(result.outcome == ServiceRequestStatusOutcome::InvalidTransition);
    CHECK(!result.serviceRequest.has_value());
    CHECK(persistedStatus(fixture, id) == "Pending");

    std::remove(databasePath.c_str());
}

// Test 6
DROGON_TEST(InProgressServiceRequestCannotReturnToPending)
{
    const std::string databasePath = "test_status_in_progress_to_pending.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    fixture.service.changeStatus(id, "In Progress");

    auto result = fixture.service.changeStatus(id, "Pending");

    CHECK(result.outcome == ServiceRequestStatusOutcome::InvalidTransition);
    CHECK(persistedStatus(fixture, id) == "In Progress");

    std::remove(databasePath.c_str());
}

// Test 7
DROGON_TEST(CompletedServiceRequestIsTerminal)
{
    const std::string databasePath = "test_status_completed_terminal.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    fixture.service.changeStatus(id, "In Progress");
    fixture.service.changeStatus(id, "Completed");

    for (const std::string target : {"Pending", "In Progress", "Cancelled"})
    {
        auto result = fixture.service.changeStatus(id, target);

        CHECK(result.outcome == ServiceRequestStatusOutcome::InvalidTransition);
        CHECK(persistedStatus(fixture, id) == "Completed");
    }

    std::remove(databasePath.c_str());
}

// Test 8
DROGON_TEST(CancelledServiceRequestIsTerminal)
{
    const std::string databasePath = "test_status_cancelled_terminal.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    fixture.service.changeStatus(id, "Cancelled");

    for (const std::string target : {"Pending", "In Progress", "Completed"})
    {
        auto result = fixture.service.changeStatus(id, target);

        CHECK(result.outcome == ServiceRequestStatusOutcome::InvalidTransition);
        CHECK(persistedStatus(fixture, id) == "Cancelled");
    }

    std::remove(databasePath.c_str());
}

// Test 9
DROGON_TEST(UnsupportedStatusIsRejected)
{
    const std::string databasePath = "test_status_unsupported.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    auto approved = fixture.service.changeStatus(id, "Approved");

    CHECK(approved.outcome == ServiceRequestStatusOutcome::UnsupportedStatus);
    CHECK(!approved.serviceRequest.has_value());
    CHECK(persistedStatus(fixture, id) == "Pending");

    // Matching is exact: wrong case and blank values are unsupported too.
    CHECK(fixture.service.changeStatus(id, "in progress").outcome
          == ServiceRequestStatusOutcome::UnsupportedStatus);
    CHECK(fixture.service.changeStatus(id, "").outcome
          == ServiceRequestStatusOutcome::UnsupportedStatus);
    CHECK(persistedStatus(fixture, id) == "Pending");

    std::remove(databasePath.c_str());
}

// Test 10
DROGON_TEST(NonexistentServiceRequestStatusChangeReturnsNotFound)
{
    const std::string databasePath = "test_status_not_found.db";
    StatusFixture fixture(databasePath);
    int existingId = addPendingRequest(fixture);

    auto result = fixture.service.changeStatus(9999, "In Progress");

    CHECK(result.outcome == ServiceRequestStatusOutcome::NotFound);
    CHECK(!result.serviceRequest.has_value());

    // Nothing was created, and the existing request is untouched.
    CHECK(!fixture.serviceRequestRepository.findById(9999).has_value());
    CHECK(!fixture.serviceRequestRepository.findById(existingId + 1).has_value());
    CHECK(persistedStatus(fixture, existingId) == "Pending");

    std::remove(databasePath.c_str());
}

// Test 11
DROGON_TEST(SuccessfulTransitionPreservesServiceRequestInformation)
{
    const std::string databasePath = "test_status_preserves_info.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    auto before = fixture.serviceRequestRepository.findById(id);

    auto result = fixture.service.changeStatus(id, "In Progress");
    CHECK(result.outcome == ServiceRequestStatusOutcome::Success);

    auto after = fixture.serviceRequestRepository.findById(id);

    CHECK(after.has_value());
    CHECK(after->getId().value() == id);
    CHECK(after->getResidentId() == before->getResidentId());
    CHECK(after->getServiceType() == "Barangay Clearance");
    CHECK(after->getDescription() == "Employment requirement");
    CHECK(after->getDateRequested() == "2026-09-15");
    CHECK(after->getStatus() == "In Progress");

    std::remove(databasePath.c_str());
}

// Test 12
DROGON_TEST(InvalidTransitionDoesNotModifyPersistence)
{
    const std::string databasePath = "test_status_invalid_no_change.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    auto before = fixture.serviceRequestRepository.findById(id);

    auto result = fixture.service.changeStatus(id, "Completed"); // Pending -> Completed
    CHECK(result.outcome == ServiceRequestStatusOutcome::InvalidTransition);

    auto after = fixture.serviceRequestRepository.findById(id);

    CHECK(after.has_value());
    CHECK(after->getId().value() == id);
    CHECK(after->getResidentId() == before->getResidentId());
    CHECK(after->getServiceType() == before->getServiceType());
    CHECK(after->getDescription() == before->getDescription());
    CHECK(after->getDateRequested() == before->getDateRequested());
    CHECK(after->getStatus() == "Pending");

    std::remove(databasePath.c_str());
}

// Test 13
DROGON_TEST(SameStatusRequestIsRejected)
{
    const std::string databasePath = "test_status_same_status.db";
    StatusFixture fixture(databasePath);
    int id = addPendingRequest(fixture);

    // Pending -> Pending
    CHECK(fixture.service.changeStatus(id, "Pending").outcome
          == ServiceRequestStatusOutcome::InvalidTransition);
    CHECK(persistedStatus(fixture, id) == "Pending");

    // In Progress -> In Progress
    fixture.service.changeStatus(id, "In Progress");
    CHECK(fixture.service.changeStatus(id, "In Progress").outcome
          == ServiceRequestStatusOutcome::InvalidTransition);
    CHECK(persistedStatus(fixture, id) == "In Progress");

    // Completed -> Completed
    fixture.service.changeStatus(id, "Completed");
    CHECK(fixture.service.changeStatus(id, "Completed").outcome
          == ServiceRequestStatusOutcome::InvalidTransition);
    CHECK(persistedStatus(fixture, id) == "Completed");

    // Cancelled -> Cancelled (on a second request)
    int secondId = addPendingRequest(fixture);
    fixture.service.changeStatus(secondId, "Cancelled");
    CHECK(fixture.service.changeStatus(secondId, "Cancelled").outcome
          == ServiceRequestStatusOutcome::InvalidTransition);
    CHECK(persistedStatus(fixture, secondId) == "Cancelled");

    std::remove(databasePath.c_str());
}

// Student-designed test
DROGON_TEST(ExistingServiceRequestKeepsProcessingAfterResidentIsDeactivated)
{
    const std::string databasePath = "test_status_resident_deactivated.db";
    StatusFixture fixture(databasePath);
    ResidentDeactivationService deactivationService(fixture.residentRepository);

    int id = addPendingRequest(fixture);
    int residentId = fixture.serviceRequestRepository.findById(id)->getResidentId();

    // The Resident is deactivated AFTER the request was submitted.
    auto deactivation = deactivationService.deactivate(residentId);
    CHECK(deactivation.outcome == ResidentDeactivationOutcome::Deactivated);

    // The existing request can still follow the full workflow.
    auto toInProgress = fixture.service.changeStatus(id, "In Progress");
    CHECK(toInProgress.outcome == ServiceRequestStatusOutcome::Success);

    auto toCompleted = fixture.service.changeStatus(id, "Completed");
    CHECK(toCompleted.outcome == ServiceRequestStatusOutcome::Success);
    CHECK(persistedStatus(fixture, id) == "Completed");

    // The request is still linked to the same Resident...
    CHECK(fixture.serviceRequestRepository.findById(id)->getResidentId() == residentId);

    // ...and T10 did not modify the Resident in any way.
    auto resident = fixture.residentRepository.findById(residentId);
    CHECK(resident.has_value());
    CHECK(resident->getStatus() == "Inactive");
    CHECK(resident->getFirstName() == "Juan");
    CHECK(resident->getLastName() == "Cruz");
    CHECK(resident->getEmail() == "juan@example.com");

    std::remove(databasePath.c_str());
}
