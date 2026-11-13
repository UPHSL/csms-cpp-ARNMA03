#include <drogon/drogon_test.h>
#include <json/json.h>

#include <sstream>
#include <string>

#include "HttpTestSupport.h"
#include "../app/AppInfo.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"

// ---- T16 Final Examination ----

namespace
{
bool contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

std::string row(const std::string& label, int value)
{
    return "<th>" + label + "</th><td>" + std::to_string(value) + "</td>";
}

std::string idPath(int id)
{
    return "/service-requests/" + std::to_string(id);
}

Json::Value parseJson(const std::string& body)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream input(body);
    if (!Json::parseFromStream(builder, input, &root, &errors))
    {
        return Json::Value();
    }
    return root;
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

int residentIdByFirstName(TestAppFixture& fixture, const std::string& firstName)
{
    const auto found = fixture.context().residentQuery().searchResidents(firstName);
    return found.size() == 1 ? found[0].getId().value_or(0) : 0;
}

int requestIdByType(TestAppFixture& fixture, const std::string& serviceType)
{
    const auto found =
        fixture.context().serviceRequestQuery().searchServiceRequests(serviceType);
    return found.serviceRequests.size() == 1
               ? found.serviceRequests[0].getId().value_or(0)
               : 0;
}

std::map<std::string, std::string> residentForm(
    const std::string& firstName,
    const std::string& lastName,
    const std::string& address,
    const std::string& contact,
    const std::string& email)
{
    return {{"firstName", firstName}, {"lastName", lastName}, {"address", address},
            {"contactNumber", contact}, {"email", email}};
}

std::map<std::string, std::string> requestForm(
    int residentId, const std::string& type, const std::string& date)
{
    return {{"residentId", std::to_string(residentId)}, {"serviceType", type},
            {"description", "Final exam scenario"}, {"dateRequested", date}};
}
}

// Release identity, verified through the real running /health route.
DROGON_TEST(HealthReportsCsmsIdentityVersionAndOkStatusWithoutInternals)
{
    const auto reply = httpGet("/health");
    CHECK(reply.status == 200);

    const Json::Value json = parseJson(reply.body);
    CHECK(json.isObject());
    CHECK(json["application"].asString() == "CSMS");
    CHECK(json["version"].asString() == "1.0.0");
    CHECK(json["status"].asString() == "OK");

    // The reported version is the single AppInfo value, and it is 1.0.0.
    CHECK(json["version"].asString() == std::string(AppInfo::kVersion));
    CHECK(std::string(AppInfo::kVersion) == "1.0.0");

    // No internals leak.
    CHECK(!contains(reply.body, "password"));
    CHECK(!contains(reply.body, "secret"));
    CHECK(!contains(reply.body, "token"));
    CHECK(!contains(reply.body, ".db"));
    CHECK(!contains(reply.body, "sqlite"));
    CHECK(!contains(reply.body, "data/"));
}

// The same version appears in the browser: home page and every module page.
DROGON_TEST(ReleaseVersionIsShownOnHomePageAndEveryModulePage)
{
    TestAppFixture fixture("t16_footer_version_test.db");

    const auto home = httpGet("/");
    CHECK(home.status == 200);
    CHECK(contains(home.body, "v1.0.0"));
    CHECK(!contains(home.body, "0.1.0"));

    for (const std::string path :
         {"/dashboard", "/residents", "/residents/new", "/service-requests",
          "/service-requests/new", "/residents/99999/edit"})
    {
        const auto reply = httpGet(path);
        const bool pageResponded = reply.status == 200 || reply.status == 404;
        CHECK(pageResponded);
        CHECK(contains(reply.body, "v" + std::string(AppInfo::kVersion)));
        CHECK(contains(reply.body, "v1.0.0"));
        CHECK(!contains(reply.body, "0.1.0"));
    }
}

// Final Examination integration test.
// Modules crossed: Resident registration/update/deactivation (T04/T06/T07),
// Service Request submission + eligibility (T09/T13), controlled workflow (T10),
// search (T11), Dashboard aggregation (T14) and real SQLite persistence.
DROGON_TEST(ResidentLifecycleRequestWorkflowAndDashboardAgreeWithPersistedData)
{
    const std::string dbPath = "t16_final_integration_test.db";
    {
        TestAppFixture fixture(dbPath);

        // Starting state: empty system.
        auto page = httpGet("/dashboard");
        CHECK(contains(page.body, row("Total Residents", 0)));
        CHECK(contains(page.body, row("Total Service Requests", 0)));

        // Register two Residents through the browser route.
        CHECK(httpPostForm("/residents",
                  residentForm("Ana", "Reyes", "12 Mabini St", "09171234567",
                               "ana@example.com")).status == 303);
        CHECK(httpPostForm("/residents",
                  residentForm("Ben", "Cruz", "5 Luna St", "09181234567",
                               "ben@example.com")).status == 303);
        const int ana = residentIdByFirstName(fixture, "Ana");
        const int ben = residentIdByFirstName(fixture, "Ben");
        CHECK(ana > 0);
        CHECK(ben > 0);
        CHECK(ana != ben);

        // Update Ana: address changes, status stays Active.
        CHECK(httpPostForm("/residents/" + std::to_string(ana),
                  residentForm("Ana", "Reyes", "99 Rizal Ave", "09171234567",
                               "ana@example.com")).status == 303);

        // Submit three requests while both Residents are Active.
        CHECK(httpPostForm("/service-requests",
                  requestForm(ana, "Barangay Clearance", "2026-11-01")).status == 303);
        CHECK(httpPostForm("/service-requests",
                  requestForm(ana, "Business Permit", "2026-11-02")).status == 303);
        CHECK(httpPostForm("/service-requests",
                  requestForm(ben, "Indigency Certificate", "2026-11-03")).status == 303);
        const int clearanceId = requestIdByType(fixture, "Clearance");
        const int permitId = requestIdByType(fixture, "Permit");
        const int indigencyId = requestIdByType(fixture, "Indigency");
        CHECK(clearanceId > 0);
        CHECK(permitId > 0);
        CHECK(indigencyId > 0);

        // Workflow: Ana's clearance Pending -> In Progress -> Completed,
        // Ana's permit -> In Progress, Ben's request Pending -> Cancelled.
        CHECK(httpPostForm(idPath(clearanceId) + "/start").status == 303);
        CHECK(httpPostForm(idPath(clearanceId) + "/complete").status == 303);
        CHECK(httpPostForm(idPath(permitId) + "/start").status == 303);
        CHECK(httpPostForm(idPath(indigencyId) + "/cancel").status == 303);

        // Deactivate Ben AFTER his request exists.
        CHECK(httpPostForm("/residents/" + std::to_string(ben) + "/deactivate").status == 303);

        // Rejected operations must leave the whole database unchanged.
        const std::string before = snapshot(fixture);

        const auto inactiveSubmit = httpPostForm("/service-requests",
            requestForm(ben, "Another Request", "2026-11-04"));
        CHECK(inactiveSubmit.status == 400);
        CHECK(contains(inactiveSubmit.body,
                       "Inactive Residents cannot submit new Service Requests."));

        const auto missingSubmit = httpPostForm("/service-requests",
            requestForm(99999, "Ghost Request", "2026-11-04"));
        CHECK(missingSubmit.status == 400);
        CHECK(contains(missingSubmit.body, "Resident not found."));

        CHECK(httpPostForm(idPath(indigencyId) + "/start").status == 409);    // Cancelled is terminal
        CHECK(httpPostForm(idPath(clearanceId) + "/cancel").status == 409);   // Completed is terminal
        CHECK(httpPostForm(idPath(permitId) + "/start").status == 409);       // In Progress -> In Progress

        const auto badUpdate = httpPostForm("/residents/" + std::to_string(ana),
            residentForm("Changed", "Reyes", "Nope", "123", "ana@example.com"));
        CHECK(badUpdate.status == 400);

        CHECK(snapshot(fixture) == before);

        // The Dashboard agrees with everything above.
        page = httpGet("/dashboard");
        CHECK(page.status == 200);
        CHECK(contains(page.body, row("Total Residents", 2)));
        CHECK(contains(page.body, row("Active Residents", 1)));
        CHECK(contains(page.body, row("Inactive Residents", 1)));
        CHECK(contains(page.body, row("Total Service Requests", 3)));
        CHECK(contains(page.body, row("Open Service Requests", 1)));
        CHECK(contains(page.body, row("Pending", 0)));
        CHECK(contains(page.body, row("In Progress", 1)));
        CHECK(contains(page.body, row("Completed", 1)));
        CHECK(contains(page.body, row("Cancelled", 1)));

        // Search still finds Ben's cancelled request even though Ben is Inactive.
        const auto cancelled =
            httpGet("/service-requests", {{"q", "Indigency"}, {"status", "Cancelled"}});
        CHECK(contains(cancelled.body, "Indigency Certificate"));
        CHECK(!contains(cancelled.body, "Business Permit"));

        // Final persisted state, read through a SECOND independent connection.
        Database secondConnection(dbPath);
        secondConnection.initialize();
        ResidentRepository residents(secondConnection);
        ServiceRequestRepository requests(secondConnection);

        const auto storedAna = residents.findById(ana);
        const auto storedBen = residents.findById(ben);
        CHECK(storedAna.has_value());
        CHECK(storedBen.has_value());
        if (storedAna && storedBen)
        {
            CHECK(storedAna->getAddress() == "99 Rizal Ave");
            CHECK(storedAna->getFirstName() == "Ana");   // bad update did not leak
            CHECK(storedAna->getContactNumber() == "09171234567");
            CHECK(storedAna->getStatus() == "Active");
            CHECK(storedBen->getStatus() == "Inactive");
        }

        CHECK(requests.countAll() == 3);
        CHECK(requests.findById(clearanceId)->getStatus() == "Completed");
        CHECK(requests.findById(permitId)->getStatus() == "In Progress");
        CHECK(requests.findById(indigencyId)->getStatus() == "Cancelled");
        CHECK(requests.findById(indigencyId)->getResidentId() == ben);
    }
}