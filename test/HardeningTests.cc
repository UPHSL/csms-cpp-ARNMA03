#include <drogon/drogon_test.h>

#include <map>
#include <string>
#include <vector>

#include "HttpTestSupport.h"

// ---- T15 hardening tests ----

namespace
{
bool contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

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

// One metric row exactly as Dashboard.csp renders it.
std::string row(const std::string& label, int value)
{
    return "<th>" + label + "</th><td>" + std::to_string(value) + "</td>";
}

// The Recent Service Requests part of the page only.
std::string recentSection(const std::string& body)
{
    const auto start = body.find("Recent Service Requests");
    return start == std::string::npos ? std::string() : body.substr(start);
}

std::string idPath(int id)
{
    return "/service-requests/" + std::to_string(id);
}
}

// Category 1 - Invalid or malformed input
DROGON_TEST(ServiceRequestUiRejectsOutOfRangeAndSignedResidentIds)
{
    TestAppFixture fixture("t15_bad_resident_id_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    const std::string before = databaseSnapshot(fixture);

    // Overflow-length, negative, decimal, explicit plus sign, and zero.
    for (const std::string badId : {"9999999999", "-5", "1.5", "+3", "0"})
    {
        const auto reply = httpPostForm("/service-requests", {
            {"residentId", badId},
            {"serviceType", "Barangay Clearance"},
            {"description", "For work"},
            {"dateRequested", "2026-11-03"}});

        CHECK(reply.status == 400);
        CHECK(contains(reply.body, "Resident ID must be a positive whole number."));
        CHECK(!contains(reply.body, "sqlite"));
        CHECK(databaseSnapshot(fixture) == before);
    }
}

// Category 2 - Rejected mutation protects persistence
DROGON_TEST(RejectedOperationsLeaveWholeDatabaseSnapshotUnchanged)
{
    TestAppFixture fixture("t15_rejected_snapshot_test.db");
    const int ana = registerResident(fixture, "Ana");
    const int ben = registerResident(fixture, "Ben");
    CHECK(ana > 0);
    CHECK(ben > 0);
    fixture.context().residentDeactivation().deactivate(ben);
    const int requestId =
        seedRequest(fixture, ana, "Barangay Clearance", "2026-11-01");
    CHECK(requestId > 0);

    const std::string before = databaseSnapshot(fixture);

    // 1. Valid first name + invalid contact number: nothing may be partially saved.
    const auto badUpdate = httpPostForm("/residents/" + std::to_string(ana), {
        {"firstName", "Changed"},
        {"lastName", "Reyes"},
        {"address", "New Address"},
        {"contactNumber", "123"},
        {"email", "ana@example.com"}});
    CHECK(badUpdate.status == 400);
    CHECK(databaseSnapshot(fixture) == before);

    const auto stored = fixture.context().residentQuery().findResident(ana);
    CHECK(stored.has_value());
    if (stored)
    {
        CHECK(stored->getFirstName() == "Ana");
        CHECK(stored->getAddress() == "12 Mabini St");
        CHECK(stored->getContactNumber() == "09171234567");
        CHECK(stored->getStatus() == "Active");
    }

    // 2. Submission for an Inactive Resident.
    const auto inactive = httpPostForm("/service-requests", {
        {"residentId", std::to_string(ben)},
        {"serviceType", "Permit"},
        {"description", "x"},
        {"dateRequested", "2026-11-02"}});
    CHECK(inactive.status == 400);
    CHECK(databaseSnapshot(fixture) == before);

    // 3. Pending -> Completed is not allowed.
    CHECK(httpPostForm(idPath(requestId) + "/complete").status == 409);
    CHECK(databaseSnapshot(fixture) == before);

    // 4. Unsupported status at the service boundary.
    CHECK(fixture.context().serviceRequestStatus()
              .changeStatus(requestId, "Approved").outcome
          == ServiceRequestStatusOutcome::UnsupportedStatus);
    CHECK(databaseSnapshot(fixture) == before);

    // 5. Missing records.
    CHECK(httpPostForm("/residents/99999", {
        {"firstName", "A"}, {"lastName", "B"}, {"address", "C"},
        {"contactNumber", "09171234567"}, {"email", "a@b.co"}}).status == 404);
    CHECK(httpPostForm("/service-requests/99999/start").status == 404);
    CHECK(databaseSnapshot(fixture) == before);
}

// Category 3 - Read-only requests do not mutate persistence
DROGON_TEST(ReadOnlyGetsAcrossAllModulesDoNotMutatePersistence)
{
    TestAppFixture fixture("t15_readonly_all_test.db");
    const int ana = registerResident(fixture, "Ana");
    const int ben = registerResident(fixture, "Ben");
    CHECK(ana > 0);
    CHECK(ben > 0);
    fixture.context().residentDeactivation().deactivate(ben);

    const int pendingId = seedRequest(fixture, ana, "PendingJob", "2026-11-01");
    const int progressId = seedRequest(fixture, ana, "ProgressJob", "2026-11-02");
    const int doneId = seedRequest(fixture, ana, "DoneJob", "2026-11-03");
    CHECK(pendingId > 0);
    CHECK(progressId > 0);
    CHECK(doneId > 0);
    CHECK(moveTo(fixture, progressId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));

    const std::string before = databaseSnapshot(fixture);

    struct ReadRequest
    {
        std::string path;
        std::map<std::string, std::string> params;
        int expectedStatus;
    };

    const std::vector<ReadRequest> reads = {
        {"/dashboard", {}, 200},
        {"/residents", {}, 200},
        {"/residents", {{"q", "Reyes"}}, 200},
        {"/residents/new", {}, 200},
        {"/residents/" + std::to_string(ana) + "/edit", {}, 200},
        {"/service-requests", {}, 200},
        {"/service-requests", {{"q", "Job"}}, 200},
        {"/service-requests", {{"status", "Pending"}}, 200},
        {"/service-requests", {{"q", "Job"}, {"status", "Completed"}}, 200},
        {"/service-requests", {{"status", "Bogus"}}, 400},
        {"/service-requests/new", {}, 200},
        {idPath(pendingId), {}, 200},
        {idPath(doneId), {}, 200},
        {"/residents/99999/edit", {}, 404},
        {"/residents/abc/edit", {}, 404},
        {idPath(99999), {}, 404},
        {"/service-requests/abc", {}, 404},
        {idPath(pendingId) + "/start", {}, 404},
        {idPath(pendingId) + "/cancel", {}, 404}};

    // Repeated on purpose: reading twice must still change nothing.
    for (int round = 0; round < 2; ++round)
    {
        for (const auto& read : reads)
        {
            CHECK(httpGet(read.path, read.params).status == read.expectedStatus);
        }
        // GET must never reach the deactivate action.
        CHECK(httpGet("/residents/" + std::to_string(ana) + "/deactivate").status >= 400);
        CHECK(databaseSnapshot(fixture) == before);
    }
}

// Category 4 - Cross-feature regression (T07 + T09 + T10 + T11 + T13 + T14)
DROGON_TEST(DeactivatedResidentFlowStaysConsistentAcrossModules)
{
    TestAppFixture fixture("t15_cross_feature_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(ana > 0);
    const int id = seedRequest(fixture, ana, "Barangay Clearance", "2026-11-01");
    CHECK(id > 0);

    auto page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Active Residents", 1)));
    CHECK(contains(page.body, row("Inactive Residents", 0)));
    CHECK(contains(page.body, row("Pending", 1)));

    // Deactivate the Resident through the browser route.
    CHECK(httpPostForm("/residents/" + std::to_string(ana) + "/deactivate").status == 303);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Total Residents", 1)));
    CHECK(contains(page.body, row("Active Residents", 0)));
    CHECK(contains(page.body, row("Inactive Residents", 1)));
    CHECK(contains(page.body, row("Total Service Requests", 1)));

    // New submissions for the Inactive Resident are rejected.
    const auto submit = httpPostForm("/service-requests", {
        {"residentId", std::to_string(ana)},
        {"serviceType", "Another"},
        {"description", "x"},
        {"dateRequested", "2026-11-02"}});
    CHECK(submit.status == 400);
    CHECK(contains(submit.body, "Inactive Residents cannot submit new Service Requests."));
    CHECK(contains(httpGet("/dashboard").body, row("Total Service Requests", 1)));

    // The existing request can still follow its workflow.
    CHECK(httpPostForm(idPath(id) + "/start").status == 303);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Pending", 0)));
    CHECK(contains(page.body, row("In Progress", 1)));
    CHECK(contains(page.body, row("Open Service Requests", 1)));

    CHECK(httpPostForm(idPath(id) + "/complete").status == 303);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Completed", 1)));
    CHECK(contains(page.body, row("Open Service Requests", 0)));

    // Everything stays visible and queryable.
    CHECK(contains(httpGet("/residents", {{"q", "Reyes"}}).body, "Inactive"));
    const auto search = httpGet("/service-requests", {{"q", "Clearance"}});
    CHECK(contains(search.body, "Barangay Clearance"));
    CHECK(contains(search.body, "Completed"));

    const auto stored = fixture.context().serviceRequestQuery().viewServiceRequest(id);
    CHECK(stored.has_value());
    if (stored)
    {
        CHECK(stored->getResidentId() == ana);
        CHECK(stored->getStatus() == "Completed");
    }
}

// Category 5 - Boundary behavior (recent list: exactly 5, then 6)
DROGON_TEST(DashboardRecentListBoundaryAtFiveAndSix)
{
    TestAppFixture fixture("t15_recent_boundary_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(ana > 0);

    for (int day = 1; day <= 5; ++day)
    {
        CHECK(seedRequest(fixture, ana, "BoundaryJob" + std::to_string(day),
                          "2026-10-0" + std::to_string(day)) > 0);
    }

    // Exactly five: nothing is cut off.
    auto page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Total Service Requests", 5)));
    std::string recent = recentSection(page.body);
    CHECK(!recent.empty());
    for (int day = 1; day <= 5; ++day)
    {
        CHECK(contains(recent, "BoundaryJob" + std::to_string(day)));
    }

    // A sixth, newest request pushes out only the oldest one.
    CHECK(seedRequest(fixture, ana, "BoundaryJob6", "2026-10-06") > 0);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Total Service Requests", 6)));
    recent = recentSection(page.body);
    CHECK(!contains(recent, "BoundaryJob1"));
    CHECK(contains(recent, "BoundaryJob2"));
    CHECK(contains(recent, "BoundaryJob6"));
    CHECK(recent.find("BoundaryJob6") < recent.find("BoundaryJob5"));

    // The cut-off request is still persisted and searchable.
    CHECK(contains(httpGet("/service-requests", {{"q", "BoundaryJob1"}}).body,
                   "BoundaryJob1"));
}