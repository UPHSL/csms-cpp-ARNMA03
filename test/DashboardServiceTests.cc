#include <drogon/drogon_test.h>
#include <cstdio>
#include <string>
#include <vector>
#include "HttpTestSupport.h"
#include "../database/Database.h"
#include "../models/Resident.h"
#include "../models/ServiceRequest.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"

namespace
{
int registerResident(TestAppFixture& fixture, const std::string& firstName)
{
    const Resident resident(
        firstName, "Reyes", "12 Mabini St", "09171234567", "resident@example.com");
    const auto result =
        fixture.context().residentRegistration().registerResident(resident);
    if (!result.success || !result.resident || !result.resident->getId())
    {
        return 0;
    }
    return *result.resident->getId();
}

int seedRequest(
    TestAppFixture& fixture,
    int residentId,
    const std::string& serviceType,
    const std::string& dateRequested)
{
    const ServiceRequest request(residentId, serviceType, "details", dateRequested);
    const auto result =
        fixture.context().serviceRequestSubmission().submit(request);
    if (result.outcome != ServiceRequestSubmissionOutcome::Success ||
        !result.serviceRequest || !result.serviceRequest->getId())
    {
        return 0;
    }
    return *result.serviceRequest->getId();
}

bool moveTo(TestAppFixture& fixture, int id, const std::string& status)
{
    return fixture.context().serviceRequestStatus().changeStatus(id, status)
               .outcome == ServiceRequestStatusOutcome::Success;
}

std::vector<int> idsOf(const std::vector<ServiceRequest>& requests)
{
    std::vector<int> ids;
    for (const auto& request : requests)
    {
        ids.push_back(request.getId().value_or(0));
    }
    return ids;
}

std::string snapshot(TestAppFixture& fixture)
{
    std::string text;
    for (const auto& r : fixture.context().residentQuery().listResidents())
    {
        text += std::to_string(r.getId().value_or(0)) + "|" + r.getFirstName() + "|" +
                r.getLastName() + "|" + r.getAddress() + "|" + r.getContactNumber() +
                "|" + r.getEmail() + "|" + r.getStatus() + ";";
    }
    text += "#";
    for (const auto& q : fixture.context().serviceRequestQuery().listServiceRequests())
    {
        text += std::to_string(q.getId().value_or(0)) + "|" +
                std::to_string(q.getResidentId()) + "|" + q.getServiceType() + "|" +
                q.getDescription() + "|" + q.getDateRequested() + "|" +
                q.getStatus() + ";";
    }
    return text;
}
}

// ---- T14 repository tests ----

DROGON_TEST(DashboardRepoResidentCountsOnEmptyDatabase)
{
    const std::string databasePath = "test_dash_repo_res_empty.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ResidentRepository repository(database);

        CHECK(repository.countAll() == 0);
        CHECK(repository.countByStatus("Active") == 0);
        CHECK(repository.countByStatus("Inactive") == 0);
    }
    std::remove(databasePath.c_str());
}

DROGON_TEST(DashboardRepoResidentCountsByStatus)
{
    const std::string databasePath = "test_dash_repo_res_counts.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ResidentRepository repository(database);

        repository.save(Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com", "Active"));
        repository.save(Resident("Ben", "Cruz", "Addr", "09172222222", "ben@example.com", "Active"));
        repository.save(Resident("Cat", "Lim", "Addr", "09173333333", "cat@example.com", "Inactive"));

        CHECK(repository.countAll() == 3);
        CHECK(repository.countByStatus("Active") == 2);
        CHECK(repository.countByStatus("Inactive") == 1);
        CHECK(repository.countByStatus("Unknown") == 0);
    }
    std::remove(databasePath.c_str());
}

DROGON_TEST(DashboardRepoServiceRequestCountsOnEmptyDatabase)
{
    const std::string databasePath = "test_dash_repo_sr_empty.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ServiceRequestRepository repository(database);

        CHECK(repository.countAll() == 0);
        CHECK(repository.countByStatus("Pending") == 0);
        CHECK(repository.countByStatus("In Progress") == 0);
        CHECK(repository.countByStatus("Completed") == 0);
        CHECK(repository.countByStatus("Cancelled") == 0);
    }
    std::remove(databasePath.c_str());
}

DROGON_TEST(DashboardRepoServiceRequestCountsByStatus)
{
    const std::string databasePath = "test_dash_repo_sr_counts.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ResidentRepository residentRepository(database);
        ServiceRequestRepository repository(database);
        const int residentId = residentRepository.save(
            Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com")).getId().value();

        repository.save(ServiceRequest(residentId, "A", "d", "2026-10-01", "Pending"));
        repository.save(ServiceRequest(residentId, "B", "d", "2026-10-02", "Pending"));
        repository.save(ServiceRequest(residentId, "C", "d", "2026-10-03", "In Progress"));
        repository.save(ServiceRequest(residentId, "D", "d", "2026-10-04", "Completed"));
        repository.save(ServiceRequest(residentId, "E", "d", "2026-10-05", "Cancelled"));

        CHECK(repository.countAll() == 5);
        CHECK(repository.countByStatus("Pending") == 2);
        CHECK(repository.countByStatus("In Progress") == 1);
        CHECK(repository.countByStatus("Completed") == 1);
        CHECK(repository.countByStatus("Cancelled") == 1);
    }
    std::remove(databasePath.c_str());
}

DROGON_TEST(DashboardRepoFindRecentLimitsAndOrders)
{
    const std::string databasePath = "test_dash_repo_recent.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ResidentRepository residentRepository(database);
        ServiceRequestRepository repository(database);
        const int residentId = residentRepository.save(
            Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com")).getId().value();

        repository.save(ServiceRequest(residentId, "A", "d", "2026-10-01"));
        const int b = repository.save(ServiceRequest(residentId, "B", "d", "2026-10-05")).getId().value();
        const int c = repository.save(ServiceRequest(residentId, "C", "d", "2026-10-03")).getId().value();
        const int d = repository.save(ServiceRequest(residentId, "D", "d", "2026-10-05")).getId().value();
        const int e = repository.save(ServiceRequest(residentId, "E", "d", "2026-10-02")).getId().value();
        const int f = repository.save(ServiceRequest(residentId, "F", "d", "2026-10-04")).getId().value();

        // date DESC then id DESC: D, B (10-05), F (10-04), C (10-03), E (10-02). A is cut.
        CHECK(idsOf(repository.findRecent(5)) == std::vector<int>({d, b, f, c, e}));
        CHECK(idsOf(repository.findRecent(2)) == std::vector<int>({d, b}));
    }
    std::remove(databasePath.c_str());
}

DROGON_TEST(DashboardRepoFindRecentWithNonPositiveLimitReturnsEmpty)
{
    const std::string databasePath = "test_dash_repo_recent_zero.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ResidentRepository residentRepository(database);
        ServiceRequestRepository repository(database);
        const int residentId = residentRepository.save(
            Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com")).getId().value();
        repository.save(ServiceRequest(residentId, "A", "d", "2026-10-01"));

        CHECK(repository.findRecent(0).empty());
        CHECK(repository.findRecent(-1).empty());
        CHECK(repository.findRecent(5).size() == 1u);
    }
    std::remove(databasePath.c_str());
}

DROGON_TEST(DashboardRepoFindRecentPreservesRequestInformation)
{
    const std::string databasePath = "test_dash_repo_recent_info.db";
    std::remove(databasePath.c_str());
    {
        Database database(databasePath);
        database.initialize();
        ResidentRepository residentRepository(database);
        ServiceRequestRepository repository(database);
        const int residentId = residentRepository.save(
            Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com")).getId().value();
        const int id = repository.save(ServiceRequest(
            residentId, "Barangay Clearance", "For work", "2026-10-15", "Completed")).getId().value();

        const auto recent = repository.findRecent(5);
        CHECK(recent.size() == 1u);
        if (recent.size() == 1u)
        {
            CHECK(recent[0].getId().value() == id);
            CHECK(recent[0].getResidentId() == residentId);
            CHECK(recent[0].getServiceType() == "Barangay Clearance");
            CHECK(recent[0].getDescription() == "For work");
            CHECK(recent[0].getDateRequested() == "2026-10-15");
            CHECK(recent[0].getStatus() == "Completed");
        }
    }
    std::remove(databasePath.c_str());
}

// ---- T14 DashboardService tests ----

// Required test 2 (service side)
DROGON_TEST(DashboardServiceEmptyDatabaseHasAllZeroCounts)
{
    TestAppFixture fixture("dash_svc_empty_test.db");
    const auto summary = fixture.context().dashboard().getSummary();

    CHECK(summary.residentTotal == 0);
    CHECK(summary.residentActive == 0);
    CHECK(summary.residentInactive == 0);
    CHECK(summary.requestTotal == 0);
    CHECK(summary.requestPending == 0);
    CHECK(summary.requestInProgress == 0);
    CHECK(summary.requestCompleted == 0);
    CHECK(summary.requestCancelled == 0);
    CHECK(summary.requestOpen == 0);
    CHECK(summary.recentRequests.empty());
}

// Required test 3
DROGON_TEST(DashboardServiceCountsTotalResidents)
{
    TestAppFixture fixture("dash_svc_resident_total_test.db");
    CHECK(registerResident(fixture, "Ana") > 0);
    CHECK(registerResident(fixture, "Ben") > 0);
    CHECK(registerResident(fixture, "Cat") > 0);

    CHECK(fixture.context().dashboard().getSummary().residentTotal == 3);
}

// Required test 4
DROGON_TEST(DashboardServiceCountsActiveAndInactiveResidents)
{
    TestAppFixture fixture("dash_svc_resident_status_test.db");
    CHECK(registerResident(fixture, "Ana") > 0);
    CHECK(registerResident(fixture, "Ben") > 0);
    const int cat = registerResident(fixture, "Cat");
    CHECK(cat > 0);
    fixture.context().residentDeactivation().deactivate(cat);

    const auto summary = fixture.context().dashboard().getSummary();
    CHECK(summary.residentActive == 2);
    CHECK(summary.residentInactive == 1);
    CHECK(summary.residentTotal == summary.residentActive + summary.residentInactive);
}

// Required test 5
DROGON_TEST(DashboardServiceCountsTotalServiceRequests)
{
    TestAppFixture fixture("dash_svc_sr_total_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    CHECK(seedRequest(fixture, residentId, "One", "2026-10-01") > 0);
    CHECK(seedRequest(fixture, residentId, "Two", "2026-10-02") > 0);
    CHECK(seedRequest(fixture, residentId, "Three", "2026-10-03") > 0);

    CHECK(fixture.context().dashboard().getSummary().requestTotal == 3);
}

// Required test 6
DROGON_TEST(DashboardServiceCountsEachServiceRequestStatus)
{
    TestAppFixture fixture("dash_svc_sr_status_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    CHECK(seedRequest(fixture, residentId, "Pending1", "2026-10-01") > 0);
    CHECK(seedRequest(fixture, residentId, "Pending2", "2026-10-02") > 0);
    const int progressId = seedRequest(fixture, residentId, "Progress", "2026-10-03");
    const int doneId = seedRequest(fixture, residentId, "Done", "2026-10-04");
    const int voidId = seedRequest(fixture, residentId, "Void", "2026-10-05");
    CHECK(progressId > 0);
    CHECK(doneId > 0);
    CHECK(voidId > 0);
    CHECK(moveTo(fixture, progressId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto summary = fixture.context().dashboard().getSummary();
    CHECK(summary.requestPending == 2);
    CHECK(summary.requestInProgress == 1);
    CHECK(summary.requestCompleted == 1);
    CHECK(summary.requestCancelled == 1);
}

// Required test 7
DROGON_TEST(DashboardServiceStatusCountsAddUpToTotal)
{
    TestAppFixture fixture("dash_svc_sr_sum_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    CHECK(seedRequest(fixture, residentId, "Pending1", "2026-10-01") > 0);
    const int progressId = seedRequest(fixture, residentId, "Progress", "2026-10-02");
    const int doneId = seedRequest(fixture, residentId, "Done", "2026-10-03");
    const int voidId = seedRequest(fixture, residentId, "Void", "2026-10-04");
    CHECK(moveTo(fixture, progressId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto s = fixture.context().dashboard().getSummary();
    CHECK(s.requestTotal == 4);
    CHECK(s.requestTotal ==
          s.requestPending + s.requestInProgress + s.requestCompleted + s.requestCancelled);
}

// Required test 8
DROGON_TEST(DashboardServiceOpenIsPendingPlusInProgressOnly)
{
    TestAppFixture fixture("dash_svc_open_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    CHECK(seedRequest(fixture, residentId, "Pending1", "2026-10-01") > 0);
    CHECK(seedRequest(fixture, residentId, "Pending2", "2026-10-02") > 0);
    const int progressId = seedRequest(fixture, residentId, "Progress", "2026-10-03");
    const int doneId = seedRequest(fixture, residentId, "Done", "2026-10-04");
    const int voidId = seedRequest(fixture, residentId, "Void", "2026-10-05");
    CHECK(moveTo(fixture, progressId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto s = fixture.context().dashboard().getSummary();
    // Completed and Cancelled are excluded.
    CHECK(s.requestOpen == 3);
    CHECK(s.requestOpen == s.requestPending + s.requestInProgress);
}

// Required test 9
DROGON_TEST(DashboardServiceDeactivationMovesActiveToInactiveKeepingTotal)
{
    TestAppFixture fixture("dash_svc_deactivate_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(registerResident(fixture, "Ben") > 0);
    CHECK(ana > 0);

    const auto before = fixture.context().dashboard().getSummary();
    fixture.context().residentDeactivation().deactivate(ana);
    const auto after = fixture.context().dashboard().getSummary();

    CHECK(after.residentTotal == before.residentTotal);
    CHECK(after.residentActive == before.residentActive - 1);
    CHECK(after.residentInactive == before.residentInactive + 1);
}

// Required test 10
DROGON_TEST(DashboardServiceStartingRequestMovesPendingToInProgress)
{
    TestAppFixture fixture("dash_svc_start_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Job", "2026-10-01");
    CHECK(id > 0);

    const auto before = fixture.context().dashboard().getSummary();
    CHECK(moveTo(fixture, id, "In Progress"));
    const auto after = fixture.context().dashboard().getSummary();

    CHECK(after.requestPending == before.requestPending - 1);
    CHECK(after.requestInProgress == before.requestInProgress + 1);
    CHECK(after.requestTotal == before.requestTotal);
    CHECK(after.requestOpen == before.requestOpen);
}

// Required test 11
DROGON_TEST(DashboardServiceCompletingOrCancellingReducesOpenKeepingTotal)
{
    TestAppFixture fixture("dash_svc_close_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int a = seedRequest(fixture, residentId, "JobA", "2026-10-01");
    const int b = seedRequest(fixture, residentId, "JobB", "2026-10-02");
    const int c = seedRequest(fixture, residentId, "JobC", "2026-10-03");
    CHECK(a > 0);
    CHECK(b > 0);
    CHECK(c > 0);
    CHECK(moveTo(fixture, a, "In Progress"));
    CHECK(moveTo(fixture, b, "In Progress"));

    const auto before = fixture.context().dashboard().getSummary();
    CHECK(before.requestOpen == 3);

    CHECK(moveTo(fixture, a, "Completed"));
    const auto afterComplete = fixture.context().dashboard().getSummary();
    CHECK(afterComplete.requestOpen == 2);
    CHECK(afterComplete.requestCompleted == 1);
    CHECK(afterComplete.requestTotal == before.requestTotal);

    CHECK(moveTo(fixture, b, "Cancelled"));
    const auto afterCancel = fixture.context().dashboard().getSummary();
    CHECK(afterCancel.requestOpen == 1);
    CHECK(afterCancel.requestCancelled == 1);
    CHECK(afterCancel.requestTotal == before.requestTotal);
}

// Required test 12
DROGON_TEST(DashboardServiceRecentListIsLimitedToFive)
{
    TestAppFixture fixture("dash_svc_recent_limit_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    for (int day = 1; day <= 7; ++day)
    {
        CHECK(seedRequest(fixture, residentId, "Job" + std::to_string(day),
                          "2026-10-0" + std::to_string(day)) > 0);
    }

    const auto summary = fixture.context().dashboard().getSummary();
    CHECK(summary.requestTotal == 7);
    CHECK(summary.recentRequests.size() == 5u);
}

// Required test 13
DROGON_TEST(DashboardServiceRecentOrderIsDateDescThenIdDesc)
{
    TestAppFixture fixture("dash_svc_recent_order_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    const int a = seedRequest(fixture, residentId, "A", "2026-10-01");
    const int b = seedRequest(fixture, residentId, "B", "2026-10-05");
    const int c = seedRequest(fixture, residentId, "C", "2026-10-03");
    const int d = seedRequest(fixture, residentId, "D", "2026-10-05"); // same date as B, higher id
    const int e = seedRequest(fixture, residentId, "E", "2026-10-02");

    const auto summary = fixture.context().dashboard().getSummary();
    CHECK(idsOf(summary.recentRequests) == std::vector<int>({d, b, c, e, a}));
}

// Required test 14
DROGON_TEST(DashboardServiceRecentListIncludesCompletedAndCancelled)
{
    TestAppFixture fixture("dash_svc_recent_terminal_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    const int doneId = seedRequest(fixture, residentId, "Done", "2026-10-01");
    const int voidId = seedRequest(fixture, residentId, "Void", "2026-10-02");
    const int openId = seedRequest(fixture, residentId, "Open", "2026-10-03");
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto ids = idsOf(fixture.context().dashboard().getSummary().recentRequests);
    CHECK(ids == std::vector<int>({openId, voidId, doneId}));
}

// Required test 15 (service side)
DROGON_TEST(DashboardServiceGetSummaryDoesNotModifyData)
{
    TestAppFixture fixture("dash_svc_readonly_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(registerResident(fixture, "Ben") > 0);
    const int id = seedRequest(fixture, residentId, "Job", "2026-10-01");
    CHECK(id > 0);
    CHECK(moveTo(fixture, id, "In Progress"));

    const std::string before = snapshot(fixture);
    fixture.context().dashboard().getSummary();
    fixture.context().dashboard().getSummary();
    fixture.context().dashboard().getSummary();

    CHECK(snapshot(fixture) == before);
}
