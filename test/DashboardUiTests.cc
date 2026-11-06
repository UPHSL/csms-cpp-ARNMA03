#include <drogon/drogon_test.h>
#include <string>
#include <vector>
#include "HttpTestSupport.h"

namespace
{
bool contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

// One metric row exactly as Dashboard.csp renders it.
std::string row(const std::string& label, int value)
{
    return "<th>" + label + "</th><td>" + std::to_string(value) + "</td>";
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

// The Recent Service Requests part of the page only, so metric labels such as
// "Pending" cannot be mistaken for table rows.
std::string recentSection(const std::string& body)
{
    const auto start = body.find("Recent Service Requests");
    return start == std::string::npos ? std::string() : body.substr(start);
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

// Required test 1
DROGON_TEST(DashboardUiPageLoadsWithNavigation)
{
    TestAppFixture fixture("dash_ui_loads_test.db");
    const auto reply = httpGet("/dashboard");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "CSMS Dashboard"));
    CHECK(contains(reply.body, "Residents"));
    CHECK(contains(reply.body, "Service Requests"));
    CHECK(contains(reply.body, "Recent Service Requests"));
    CHECK(contains(reply.body, R"(<a href="/dashboard">Dashboard</a>)"));
    CHECK(contains(reply.body, R"(<a href="/residents">Residents</a>)"));
    CHECK(contains(reply.body, R"(<a href="/service-requests">Service Requests</a>)"));

    // The Dashboard link is shared layout, so other pages carry it too.
    CHECK(contains(httpGet("/residents").body, R"(<a href="/dashboard">Dashboard</a>)"));
    CHECK(contains(httpGet("/service-requests").body, R"(<a href="/dashboard">Dashboard</a>)"));
}

// Required test 2 (rendered)
DROGON_TEST(DashboardUiEmptyDatabaseShowsZerosAndEmptyState)
{
    TestAppFixture fixture("dash_ui_empty_test.db");
    const auto reply = httpGet("/dashboard");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, row("Total Residents", 0)));
    CHECK(contains(reply.body, row("Active Residents", 0)));
    CHECK(contains(reply.body, row("Inactive Residents", 0)));
    CHECK(contains(reply.body, row("Total Service Requests", 0)));
    CHECK(contains(reply.body, row("Open Service Requests", 0)));
    CHECK(contains(reply.body, row("Pending", 0)));
    CHECK(contains(reply.body, row("In Progress", 0)));
    CHECK(contains(reply.body, row("Completed", 0)));
    CHECK(contains(reply.body, row("Cancelled", 0)));
    CHECK(contains(reply.body, "No Service Requests available."));
}

// Required tests 3-8 (rendered)
DROGON_TEST(DashboardUiShowsResidentAndRequestMetrics)
{
    TestAppFixture fixture("dash_ui_metrics_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(registerResident(fixture, "Ben") > 0);
    const int cat = registerResident(fixture, "Cat");
    CHECK(ana > 0);
    CHECK(cat > 0);
    fixture.context().residentDeactivation().deactivate(cat);

    CHECK(seedRequest(fixture, ana, "Pending1", "2026-10-01") > 0);
    CHECK(seedRequest(fixture, ana, "Pending2", "2026-10-02") > 0);
    const int progressId = seedRequest(fixture, ana, "Progress", "2026-10-03");
    const int doneId = seedRequest(fixture, ana, "Done", "2026-10-04");
    const int voidId = seedRequest(fixture, ana, "Void", "2026-10-05");
    CHECK(moveTo(fixture, progressId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto reply = httpGet("/dashboard");
    CHECK(reply.status == 200);
    CHECK(contains(reply.body, row("Total Residents", 3)));
    CHECK(contains(reply.body, row("Active Residents", 2)));
    CHECK(contains(reply.body, row("Inactive Residents", 1)));
    CHECK(contains(reply.body, row("Total Service Requests", 5)));
    CHECK(contains(reply.body, row("Open Service Requests", 3)));
    CHECK(contains(reply.body, row("Pending", 2)));
    CHECK(contains(reply.body, row("In Progress", 1)));
    CHECK(contains(reply.body, row("Completed", 1)));
    CHECK(contains(reply.body, row("Cancelled", 1)));
    CHECK(!contains(reply.body, "No Service Requests available."));
}

// Required test 9 (rendered)
DROGON_TEST(DashboardUiReflectsResidentDeactivation)
{
    TestAppFixture fixture("dash_ui_deactivate_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(registerResident(fixture, "Ben") > 0);
    CHECK(ana > 0);

    const auto before = httpGet("/dashboard");
    CHECK(contains(before.body, row("Total Residents", 2)));
    CHECK(contains(before.body, row("Active Residents", 2)));
    CHECK(contains(before.body, row("Inactive Residents", 0)));

    CHECK(httpPostForm("/residents/" + std::to_string(ana) + "/deactivate").status == 303);

    const auto after = httpGet("/dashboard");
    CHECK(contains(after.body, row("Total Residents", 2)));
    CHECK(contains(after.body, row("Active Residents", 1)));
    CHECK(contains(after.body, row("Inactive Residents", 1)));
}

// Required tests 10-11 (rendered, via the real T13 POST routes)
DROGON_TEST(DashboardUiReflectsStatusChanges)
{
    TestAppFixture fixture("dash_ui_status_test.db");
    const int ana = registerResident(fixture, "Ana");
    const int a = seedRequest(fixture, ana, "JobA", "2026-10-01");
    const int b = seedRequest(fixture, ana, "JobB", "2026-10-02");
    CHECK(a > 0);
    CHECK(b > 0);

    auto page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Pending", 2)));
    CHECK(contains(page.body, row("In Progress", 0)));
    CHECK(contains(page.body, row("Open Service Requests", 2)));

    CHECK(httpPostForm("/service-requests/" + std::to_string(a) + "/start").status == 303);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Pending", 1)));
    CHECK(contains(page.body, row("In Progress", 1)));
    CHECK(contains(page.body, row("Open Service Requests", 2)));
    CHECK(contains(page.body, row("Total Service Requests", 2)));

    CHECK(httpPostForm("/service-requests/" + std::to_string(a) + "/complete").status == 303);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Completed", 1)));
    CHECK(contains(page.body, row("Open Service Requests", 1)));
    CHECK(contains(page.body, row("Total Service Requests", 2)));

    CHECK(httpPostForm("/service-requests/" + std::to_string(b) + "/cancel").status == 303);
    page = httpGet("/dashboard");
    CHECK(contains(page.body, row("Cancelled", 1)));
    CHECK(contains(page.body, row("Open Service Requests", 0)));
    CHECK(contains(page.body, row("Total Service Requests", 2)));
}

// Required tests 12-13 (rendered)
DROGON_TEST(DashboardUiRecentListIsLimitedAndOrdered)
{
    TestAppFixture fixture("dash_ui_recent_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(ana > 0);
    // Oldest (Job1) must be cut. Job5 and Job6 share a date, so Job6 (higher id) leads.
    CHECK(seedRequest(fixture, ana, "RecentJob1", "2026-10-01") > 0);
    CHECK(seedRequest(fixture, ana, "RecentJob2", "2026-10-02") > 0);
    CHECK(seedRequest(fixture, ana, "RecentJob3", "2026-10-03") > 0);
    CHECK(seedRequest(fixture, ana, "RecentJob4", "2026-10-04") > 0);
    CHECK(seedRequest(fixture, ana, "RecentJob5", "2026-10-05") > 0);
    CHECK(seedRequest(fixture, ana, "RecentJob6", "2026-10-05") > 0);

    const auto reply = httpGet("/dashboard");
    CHECK(reply.status == 200);
    CHECK(contains(reply.body, row("Total Service Requests", 6)));

    const std::string recent = recentSection(reply.body);
    CHECK(!recent.empty());
    CHECK(!contains(recent, "RecentJob1"));

    const auto job6 = recent.find("RecentJob6");
    const auto job5 = recent.find("RecentJob5");
    const auto job4 = recent.find("RecentJob4");
    const auto job3 = recent.find("RecentJob3");
    const auto job2 = recent.find("RecentJob2");
    CHECK(job6 != std::string::npos);
    CHECK(job5 != std::string::npos);
    CHECK(job4 != std::string::npos);
    CHECK(job3 != std::string::npos);
    CHECK(job2 != std::string::npos);
    CHECK(job6 < job5);
    CHECK(job5 < job4);
    CHECK(job4 < job3);
    CHECK(job3 < job2);
}

// Required test 14 (rendered) plus the View link
DROGON_TEST(DashboardUiRecentListShowsTerminalRequestsWithViewLink)
{
    TestAppFixture fixture("dash_ui_terminal_test.db");
    const int ana = registerResident(fixture, "Ana");
    const int doneId = seedRequest(fixture, ana, "DoneJob", "2026-10-01");
    const int voidId = seedRequest(fixture, ana, "VoidJob", "2026-10-02");
    CHECK(doneId > 0);
    CHECK(voidId > 0);
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto reply = httpGet("/dashboard");
    CHECK(reply.status == 200);

    const std::string recent = recentSection(reply.body);
    CHECK(contains(recent, "DoneJob"));
    CHECK(contains(recent, "VoidJob"));
    CHECK(contains(recent, "<td>Completed</td>"));
    CHECK(contains(recent, "<td>Cancelled</td>"));
    CHECK(contains(recent, "2026-10-01"));
    CHECK(contains(recent, "2026-10-02"));

    // View links go to the existing T13 details page.
    CHECK(contains(recent, "/service-requests/" + std::to_string(doneId) + R"(">View</a>)"));
    CHECK(contains(recent, "/service-requests/" + std::to_string(voidId) + R"(">View</a>)"));
    CHECK(httpGet("/service-requests/" + std::to_string(doneId)).status == 200);

    // The dashboard offers no workflow actions.
    CHECK(!contains(reply.body, "<form"));
    CHECK(!contains(reply.body, "/start"));
    CHECK(!contains(reply.body, "/complete"));
    CHECK(!contains(reply.body, "/cancel"));
}

// Required test 15 (rendered)
DROGON_TEST(DashboardUiRepeatedRefreshChangesNothing)
{
    TestAppFixture fixture("dash_ui_refresh_test.db");
    const int ana = registerResident(fixture, "Ana");
    CHECK(registerResident(fixture, "Ben") > 0);
    const int id = seedRequest(fixture, ana, "Job", "2026-10-01");
    CHECK(id > 0);
    CHECK(moveTo(fixture, id, "In Progress"));

    const std::string before = snapshot(fixture);
    const auto first = httpGet("/dashboard");
    const auto second = httpGet("/dashboard");
    const auto third = httpGet("/dashboard");

    CHECK(first.status == 200);
    CHECK(second.status == 200);
    CHECK(third.status == 200);
    CHECK(first.body == second.body);
    CHECK(second.body == third.body);
    CHECK(snapshot(fixture) == before);
}
