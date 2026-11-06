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

// Registers a resident through the T04 service; returns the id or 0.
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

// Submits through T09 and returns the generated id, or 0 on failure.
int seedRequest(
    TestAppFixture& fixture,
    int residentId,
    const std::string& serviceType,
    const std::string& description,
    const std::string& dateRequested)
{
    const ServiceRequest request(
        residentId, serviceType, description, dateRequested);
    const auto result =
        fixture.context().serviceRequestSubmission().submit(request);

    if (result.outcome != ServiceRequestSubmissionOutcome::Success ||
        !result.serviceRequest || !result.serviceRequest->getId())
    {
        return 0;
    }
    return *result.serviceRequest->getId();
}

// Moves a request through T10; true only if T10 accepted the transition.
bool moveTo(TestAppFixture& fixture, int id, const std::string& status)
{
    return fixture.context().serviceRequestStatus().changeStatus(id, status)
               .outcome == ServiceRequestStatusOutcome::Success;
}

// Current persisted status, read through T11; empty if not found.
std::string statusOf(TestAppFixture& fixture, int id)
{
    const auto found = fixture.context().serviceRequestQuery().viewServiceRequest(id);
    return found ? found->getStatus() : std::string();
}

std::string idPath(int id)
{
    return "/service-requests/" + std::to_string(id);
}

const std::string kDate = "2026-11-03";
}

// Test 1
DROGON_TEST(ServiceRequestUiManagementPageLoads)
{
    TestAppFixture fixture("sr_ui_page_loads_test.db");

    const auto reply = httpGet("/service-requests");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "Service Request Management"));
    CHECK(contains(reply.body, "Submit Service Request"));
    CHECK(contains(reply.body, R"(name="q")"));
    CHECK(contains(reply.body, R"(name="status")"));
    CHECK(contains(reply.body, "All Statuses"));
    CHECK(contains(reply.body, "No Service Requests found."));
}

// Test 2
DROGON_TEST(ServiceRequestUiListShowsPersistedRequests)
{
    TestAppFixture fixture("sr_ui_list_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    CHECK(seedRequest(fixture, residentId, "Barangay Clearance", "For work", "2026-10-01") > 0);
    CHECK(seedRequest(fixture, residentId, "Business Permit", "Small stall", "2026-10-02") > 0);

    const auto reply = httpGet("/service-requests");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "Barangay Clearance"));
    CHECK(contains(reply.body, "Business Permit"));
    CHECK(contains(reply.body, "2026-10-01"));
    CHECK(contains(reply.body, "2026-10-02"));
    CHECK(contains(reply.body, "Pending"));
    CHECK(contains(reply.body, ">View<"));
    CHECK(!contains(reply.body, "No Service Requests found."));
}

// Test 3
DROGON_TEST(ServiceRequestUiListPreservesRequiredOrdering)
{
    TestAppFixture fixture("sr_ui_ordering_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);

    // Inserted in an order that differs from the required result.
    CHECK(seedRequest(fixture, residentId, "Type Alpha", "a", "2026-01-10") > 0);
    CHECK(seedRequest(fixture, residentId, "Type Bravo", "b", "2026-03-05") > 0);
    CHECK(seedRequest(fixture, residentId, "Type Delta", "d", "2026-02-01") > 0);
    CHECK(seedRequest(fixture, residentId, "Type Charlie", "c", "2026-03-05") > 0);

    const auto reply = httpGet("/service-requests");
    CHECK(reply.status == 200);

    const auto alpha = reply.body.find("Type Alpha");
    const auto bravo = reply.body.find("Type Bravo");
    const auto charlie = reply.body.find("Type Charlie");
    const auto delta = reply.body.find("Type Delta");
    CHECK(alpha != std::string::npos);
    CHECK(bravo != std::string::npos);
    CHECK(charlie != std::string::npos);
    CHECK(delta != std::string::npos);

    // dateRequested DESC, then id DESC:
    // Charlie (03-05, higher id), Bravo (03-05), Delta (02-01), Alpha (01-10).
    CHECK(charlie < bravo);
    CHECK(bravo < delta);
    CHECK(delta < alpha);
}

// Test 4
DROGON_TEST(ServiceRequestUiSearchUsesT11Behavior)
{
    TestAppFixture fixture("sr_ui_search_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    CHECK(seedRequest(fixture, residentId, "Barangay Clearance",
                      "Requesting clearance for employment requirements.", kDate) > 0);
    CHECK(seedRequest(fixture, residentId, "Business Permit",
                      "Small stall near the market", kDate) > 0);

    const auto byType = httpGet("/service-requests", {{"q", "CLEAR"}});
    CHECK(byType.status == 200);
    CHECK(contains(byType.body, "Barangay Clearance"));
    CHECK(!contains(byType.body, "Business Permit"));

    const auto byDescription = httpGet("/service-requests", {{"q", "Employ"}});
    CHECK(contains(byDescription.body, "Barangay Clearance"));
    CHECK(!contains(byDescription.body, "Business Permit"));

    const auto padded = httpGet("/service-requests", {{"q", "   clearance   "}});
    CHECK(contains(padded.body, "Barangay Clearance"));
    CHECK(!contains(padded.body, "Business Permit"));

    const auto blank = httpGet("/service-requests", {{"q", "   "}});
    CHECK(blank.status == 200);
    CHECK(contains(blank.body, "Barangay Clearance"));
    CHECK(contains(blank.body, "Business Permit"));

    const auto none = httpGet("/service-requests", {{"q", "zzzz"}});
    CHECK(none.status == 200);
    CHECK(contains(none.body, "No Service Requests found."));
}

// Test 5
DROGON_TEST(ServiceRequestUiStatusFilterUsesT11Behavior)
{
    TestAppFixture fixture("sr_ui_status_filter_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);

    CHECK(seedRequest(fixture, residentId, "PendingJob", "p", kDate) > 0);
    const int progressId = seedRequest(fixture, residentId, "ProgressJob", "i", kDate);
    const int doneId = seedRequest(fixture, residentId, "DoneJob", "c", kDate);
    const int voidId = seedRequest(fixture, residentId, "VoidJob", "x", kDate);
    CHECK(progressId > 0);
    CHECK(doneId > 0);
    CHECK(voidId > 0);
    CHECK(moveTo(fixture, progressId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto pending = httpGet("/service-requests", {{"status", "Pending"}});
    CHECK(pending.status == 200);
    CHECK(contains(pending.body, "PendingJob"));
    CHECK(!contains(pending.body, "ProgressJob"));
    CHECK(!contains(pending.body, "DoneJob"));
    CHECK(!contains(pending.body, "VoidJob"));

    const auto inProgress = httpGet("/service-requests", {{"status", "In Progress"}});
    CHECK(contains(inProgress.body, "ProgressJob"));
    CHECK(!contains(inProgress.body, "PendingJob"));
    CHECK(!contains(inProgress.body, "DoneJob"));
    CHECK(!contains(inProgress.body, "VoidJob"));

    const auto completed = httpGet("/service-requests", {{"status", "Completed"}});
    CHECK(contains(completed.body, "DoneJob"));
    CHECK(!contains(completed.body, "PendingJob"));
    CHECK(!contains(completed.body, "ProgressJob"));
    CHECK(!contains(completed.body, "VoidJob"));

    const auto cancelled = httpGet("/service-requests", {{"status", "Cancelled"}});
    CHECK(contains(cancelled.body, "VoidJob"));
    CHECK(!contains(cancelled.body, "PendingJob"));
    CHECK(!contains(cancelled.body, "ProgressJob"));
    CHECK(!contains(cancelled.body, "DoneJob"));

    const auto all = httpGet("/service-requests", {{"status", ""}});
    CHECK(contains(all.body, "PendingJob"));
    CHECK(contains(all.body, "ProgressJob"));
    CHECK(contains(all.body, "DoneJob"));
    CHECK(contains(all.body, "VoidJob"));
}

// Test 6
DROGON_TEST(ServiceRequestUiCombinedSearchAndStatusFilterWorks)
{
    TestAppFixture fixture("sr_ui_combined_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);

    // text only: matches "clearance", but not Pending
    const int textOnly = seedRequest(fixture, residentId, "Clearance Alpha", "x", kDate);
    // status only: Pending, but no "clearance"
    CHECK(seedRequest(fixture, residentId, "Permit Beta", "y", kDate) > 0);
    // both: "clearance" and Pending
    CHECK(seedRequest(fixture, residentId, "Clearance Gamma", "z", kDate) > 0);
    // neither: no "clearance", not Pending
    const int neither = seedRequest(fixture, residentId, "Permit Delta", "w", kDate);
    CHECK(textOnly > 0);
    CHECK(neither > 0);
    CHECK(moveTo(fixture, textOnly, "In Progress"));
    CHECK(moveTo(fixture, neither, "In Progress"));

    const auto reply = httpGet("/service-requests",
        {{"q", "clearance"}, {"status", "Pending"}});

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "Clearance Gamma"));
    CHECK(!contains(reply.body, "Clearance Alpha"));
    CHECK(!contains(reply.body, "Permit Beta"));
    CHECK(!contains(reply.body, "Permit Delta"));
}

// Test 7
DROGON_TEST(ServiceRequestUiSubmissionFormShowsRequiredFields)
{
    TestAppFixture fixture("sr_ui_form_test.db");

    const auto reply = httpGet("/service-requests/new");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, R"(action="/service-requests")"));
    CHECK(contains(reply.body, R"(name="residentId")"));
    CHECK(contains(reply.body, R"(name="serviceType")"));
    CHECK(contains(reply.body, R"(name="description")"));
    CHECK(contains(reply.body, R"(name="dateRequested")"));
    CHECK(!contains(reply.body, R"(name="id")"));
    CHECK(!contains(reply.body, R"(name="status")"));
}

// Test 8
DROGON_TEST(ServiceRequestUiValidSubmissionPersistsPendingRequest)
{
    TestAppFixture fixture("sr_ui_submit_valid_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);

    const auto reply = httpPostForm("/service-requests", {
        {"residentId", std::to_string(residentId)},
        {"serviceType", "Barangay Clearance"},
        {"description", "For employment"},
        {"dateRequested", "2026-11-03"}});

    CHECK(reply.status == 303);
    CHECK(reply.location == "/service-requests?success=submitted");

    const auto requests = fixture.context().serviceRequestQuery().listServiceRequests();
    CHECK(requests.size() == 1);
    if (requests.size() == 1)
    {
        CHECK(requests[0].getId().has_value());
        CHECK(requests[0].getResidentId() == residentId);
        CHECK(requests[0].getServiceType() == "Barangay Clearance");
        CHECK(requests[0].getDescription() == "For employment");
        CHECK(requests[0].getDateRequested() == "2026-11-03");
        CHECK(requests[0].getStatus() == "Pending");
    }

    // A forged status/id in the form is ignored.
    const auto forged = httpPostForm("/service-requests", {
        {"residentId", std::to_string(residentId)},
        {"serviceType", "Indigency"},
        {"description", "Scholarship"},
        {"dateRequested", "2026-11-04"},
        {"status", "Completed"},
        {"id", "999"}});
    CHECK(forged.status == 303);

    const auto after = fixture.context().serviceRequestQuery().listServiceRequests();
    CHECK(after.size() == 2);
    for (const auto& request : after)
    {
        CHECK(request.getStatus() == "Pending");
        CHECK(request.getId().value_or(0) != 999);
    }
}

// Test 9
DROGON_TEST(ServiceRequestUiInvalidSubmissionShowsValidationFeedback)
{
    TestAppFixture fixture("sr_ui_submit_invalid_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);

    const auto reply = httpPostForm("/service-requests", {
        {"residentId", std::to_string(residentId)},
        {"serviceType", "   "},
        {"description", "Keep this text"},
        {"dateRequested", ""}});

    CHECK(reply.status == 400);
    CHECK(contains(reply.body, "Service type is required."));
    CHECK(contains(reply.body, "Date requested is required."));
    CHECK(contains(reply.body, "Keep this text"));
    CHECK(fixture.context().serviceRequestQuery().listServiceRequests().empty());

    const auto badResident = httpPostForm("/service-requests", {
        {"residentId", "abc"},
        {"serviceType", "Barangay Clearance"},
        {"description", "x"},
        {"dateRequested", kDate}});
    CHECK(badResident.status == 400);
    CHECK(contains(badResident.body, "Resident ID must be a positive whole number."));
    CHECK(fixture.context().serviceRequestQuery().listServiceRequests().empty());
}

// Test 10
DROGON_TEST(ServiceRequestUiMissingResidentSubmissionIsRejected)
{
    TestAppFixture fixture("sr_ui_submit_missing_resident_test.db");

    const auto reply = httpPostForm("/service-requests", {
        {"residentId", "99999"},
        {"serviceType", "Barangay Clearance"},
        {"description", "For work"},
        {"dateRequested", kDate}});

    CHECK(reply.status == 400);
    CHECK(contains(reply.body, "Resident not found."));
    CHECK(fixture.context().serviceRequestQuery().listServiceRequests().empty());
}

// Test 11
DROGON_TEST(ServiceRequestUiInactiveResidentSubmissionIsRejected)
{
    TestAppFixture fixture("sr_ui_submit_inactive_resident_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    fixture.context().residentDeactivation().deactivate(residentId);

    const auto reply = httpPostForm("/service-requests", {
        {"residentId", std::to_string(residentId)},
        {"serviceType", "Barangay Clearance"},
        {"description", "For work"},
        {"dateRequested", kDate}});

    CHECK(reply.status == 400);
    CHECK(contains(reply.body, "Inactive Residents cannot submit new Service Requests."));
    CHECK(fixture.context().serviceRequestQuery().listServiceRequests().empty());
}

// Test 12
DROGON_TEST(ServiceRequestUiDetailsShowAllFields)
{
    TestAppFixture fixture("sr_ui_details_test.db");
    const int residentId = registerResident(fixture, "Ana");
    CHECK(residentId > 0);
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "Needed for employment", "2026-10-15");
    CHECK(id > 0);

    const auto reply = httpGet(idPath(id));

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "<td>" + std::to_string(id) + "</td>"));
    CHECK(contains(reply.body, "<td>" + std::to_string(residentId) + "</td>"));
    CHECK(contains(reply.body, "Barangay Clearance"));
    CHECK(contains(reply.body, "Needed for employment"));
    CHECK(contains(reply.body, "2026-10-15"));
    CHECK(contains(reply.body, "<td>Pending</td>"));
    // Pending offers exactly Start Processing and Cancel.
    CHECK(contains(reply.body, idPath(id) + "/start"));
    CHECK(contains(reply.body, idPath(id) + "/cancel"));
    CHECK(!contains(reply.body, idPath(id) + "/complete"));
}

// Test 13
DROGON_TEST(ServiceRequestUiMissingRequestViewIsHandledSafely)
{
    TestAppFixture fixture("sr_ui_view_missing_test.db");

    const auto missing = httpGet("/service-requests/99999");
    CHECK(missing.status == 404);
    CHECK(contains(missing.body, "Service Request not found."));

    const auto malformed = httpGet("/service-requests/abc");
    CHECK(malformed.status == 404);

    const auto zero = httpGet("/service-requests/0");
    CHECK(zero.status == 404);

    CHECK(fixture.context().serviceRequestQuery().listServiceRequests().empty());
}

// Test 14
DROGON_TEST(ServiceRequestUiPendingRequestCanStartProcessing)
{
    TestAppFixture fixture("sr_ui_start_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "For work", "2026-10-15");
    CHECK(id > 0);

    const auto reply = httpPostForm(idPath(id) + "/start");

    CHECK(reply.status == 303);
    CHECK(reply.location == "/service-requests?success=started");

    const auto after = fixture.context().serviceRequestQuery().viewServiceRequest(id);
    CHECK(after.has_value());
    if (after)
    {
        CHECK(after->getStatus() == "In Progress");
        CHECK(after->getId().value_or(0) == id);
        CHECK(after->getResidentId() == residentId);
        CHECK(after->getServiceType() == "Barangay Clearance");
        CHECK(after->getDescription() == "For work");
        CHECK(after->getDateRequested() == "2026-10-15");
    }
}

// Test 15
DROGON_TEST(ServiceRequestUiPendingRequestCanBeCancelled)
{
    TestAppFixture fixture("sr_ui_cancel_pending_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "For work", kDate);
    CHECK(id > 0);

    const auto reply = httpPostForm(idPath(id) + "/cancel");

    CHECK(reply.status == 303);
    CHECK(reply.location == "/service-requests?success=cancelled");
    CHECK(statusOf(fixture, id) == "Cancelled");
}

// Test 16
DROGON_TEST(ServiceRequestUiInProgressRequestCanBeCompleted)
{
    TestAppFixture fixture("sr_ui_complete_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "For work", kDate);
    CHECK(id > 0);
    CHECK(moveTo(fixture, id, "In Progress"));

    const auto reply = httpPostForm(idPath(id) + "/complete");

    CHECK(reply.status == 303);
    CHECK(reply.location == "/service-requests?success=completed");
    CHECK(statusOf(fixture, id) == "Completed");
}

// Test 17
DROGON_TEST(ServiceRequestUiInProgressRequestCanBeCancelled)
{
    TestAppFixture fixture("sr_ui_cancel_in_progress_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "For work", kDate);
    CHECK(id > 0);
    CHECK(moveTo(fixture, id, "In Progress"));

    const auto reply = httpPostForm(idPath(id) + "/cancel");

    CHECK(reply.status == 303);
    CHECK(reply.location == "/service-requests?success=cancelled");
    CHECK(statusOf(fixture, id) == "Cancelled");
}

// Test 18
DROGON_TEST(ServiceRequestUiInvalidDirectTransitionIsStillRejected)
{
    TestAppFixture fixture("sr_ui_invalid_transition_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "For work", kDate);
    CHECK(id > 0);

    // The button is not offered, but a hand-made POST must still fail in T10.
    const auto reply = httpPostForm(idPath(id) + "/complete");

    CHECK(reply.status == 409);
    CHECK(contains(reply.body, "not allowed"));
    CHECK(statusOf(fixture, id) == "Pending");

    // Terminal requests cannot move again either.
    CHECK(moveTo(fixture, id, "Cancelled"));
    CHECK(httpPostForm(idPath(id) + "/start").status == 409);
    CHECK(httpPostForm(idPath(id) + "/complete").status == 409);
    CHECK(httpPostForm(idPath(id) + "/cancel").status == 409);
    CHECK(statusOf(fixture, id) == "Cancelled");

    // Unknown ids are a controlled 404.
    CHECK(httpPostForm("/service-requests/99999/start").status == 404);
    CHECK(httpPostForm("/service-requests/abc/cancel").status == 404);
}

// Test 19
DROGON_TEST(ServiceRequestUiTerminalRequestsRemainVisibleButOfferNoActions)
{
    TestAppFixture fixture("sr_ui_terminal_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int doneId = seedRequest(fixture, residentId, "DoneJob", "finished", kDate);
    const int voidId = seedRequest(fixture, residentId, "VoidJob", "dropped", kDate);
    CHECK(doneId > 0);
    CHECK(voidId > 0);
    CHECK(moveTo(fixture, doneId, "In Progress"));
    CHECK(moveTo(fixture, doneId, "Completed"));
    CHECK(moveTo(fixture, voidId, "Cancelled"));

    const auto list = httpGet("/service-requests");
    CHECK(contains(list.body, "DoneJob"));
    CHECK(contains(list.body, "VoidJob"));
    CHECK(contains(list.body, "Completed"));
    CHECK(contains(list.body, "Cancelled"));
    CHECK(!contains(list.body, "/start"));
    CHECK(!contains(list.body, "/complete"));
    CHECK(!contains(list.body, "/cancel"));

    const auto filtered = httpGet("/service-requests", {{"status", "Completed"}});
    CHECK(contains(filtered.body, "DoneJob"));
    CHECK(!contains(filtered.body, "VoidJob"));

    const auto searched = httpGet("/service-requests", {{"q", "dropped"}});
    CHECK(contains(searched.body, "VoidJob"));
    CHECK(!contains(searched.body, "DoneJob"));

    const auto doneDetails = httpGet(idPath(doneId));
    CHECK(doneDetails.status == 200);
    CHECK(contains(doneDetails.body, "<td>Completed</td>"));
    CHECK(!contains(doneDetails.body, "<button"));

    const auto voidDetails = httpGet(idPath(voidId));
    CHECK(voidDetails.status == 200);
    CHECK(contains(voidDetails.body, "<td>Cancelled</td>"));
    CHECK(!contains(voidDetails.body, "<button"));
}

// Test 20
DROGON_TEST(ServiceRequestUiGetRequestsDoNotChangeState)
{
    TestAppFixture fixture("sr_ui_get_readonly_test.db");
    const int residentId = registerResident(fixture, "Ana");
    const int id = seedRequest(fixture, residentId, "Barangay Clearance",
                               "For work", kDate);
    CHECK(id > 0);

    CHECK(httpGet("/service-requests").status == 200);
    CHECK(httpGet("/service-requests", {{"q", "clearance"}}).status == 200);
    CHECK(httpGet("/service-requests", {{"status", "Pending"}}).status == 200);
    CHECK(httpGet("/service-requests/new").status == 200);
    CHECK(httpGet(idPath(id)).status == 200);
    // GET has no route for the actions, so it cannot mutate anything.
    CHECK(httpGet(idPath(id) + "/start").status == 404);
    CHECK(httpGet(idPath(id) + "/complete").status == 404);
    CHECK(httpGet(idPath(id) + "/cancel").status == 404);

    const auto requests = fixture.context().serviceRequestQuery().listServiceRequests();
    CHECK(requests.size() == 1);
    CHECK(statusOf(fixture, id) == "Pending");
}