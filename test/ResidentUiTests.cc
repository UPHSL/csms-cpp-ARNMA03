#include <drogon/drogon_test.h>

#include <string>

#include "HttpTestSupport.h"

namespace
{
bool contains(const std::string& text, const std::string& part)
{
    return text.find(part) != std::string::npos;
}

// Registers a resident through the T04 service and returns the generated id,
// or 0 if registration failed.
int registerResident(
    TestAppFixture& fixture,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& address = "12 Mabini St",
    const std::string& contactNumber = "09171234567",
    const std::string& email = "resident@example.com")
{
    const Resident resident(firstName, lastName, address, contactNumber, email);
    const auto result =
        fixture.context().residentRegistration().registerResident(resident);

    if (!result.success || !result.resident || !result.resident->getId())
    {
        return 0;
    }
    return *result.resident->getId();
}
}

DROGON_TEST(ResidentUiEmptyListShowsNoResidentsFound)
{
    TestAppFixture fixture("resident_ui_empty_list_test.db");

    const auto reply = httpGet("/residents");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "No Residents found."));
}

DROGON_TEST(ResidentUiListShowsRegisteredResidents)
{
    TestAppFixture fixture("resident_ui_list_test.db");
    CHECK(registerResident(fixture, "Ana", "Reyes", "12 Mabini St",
                           "09171234567", "ana@example.com") > 0);
    CHECK(registerResident(fixture, "Ben", "Cruz", "5 Luna St",
                           "09181234567", "ben@example.com") > 0);

    const auto reply = httpGet("/residents");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "Ana"));
    CHECK(contains(reply.body, "Reyes"));
    CHECK(contains(reply.body, "12 Mabini St"));
    CHECK(contains(reply.body, "09171234567"));
    CHECK(contains(reply.body, "ana@example.com"));
    CHECK(contains(reply.body, "Cruz"));
    CHECK(contains(reply.body, "Active"));
    CHECK(!contains(reply.body, "No Residents found."));
}

DROGON_TEST(ResidentUiSearchShowsOnlyMatchingResidents)
{
    TestAppFixture fixture("resident_ui_search_test.db");
    CHECK(registerResident(fixture, "Ana", "Reyes") > 0);
    CHECK(registerResident(fixture, "Ben", "Cruz") > 0);

    const auto reply = httpGet("/residents", {{"q", "Reyes"}});

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, "Reyes"));
    CHECK(!contains(reply.body, "Cruz"));
}

DROGON_TEST(ResidentUiBlankSearchListsAllResidents)
{
    TestAppFixture fixture("resident_ui_blank_search_test.db");
    CHECK(registerResident(fixture, "Ana", "Reyes") > 0);
    CHECK(registerResident(fixture, "Ben", "Cruz") > 0);

    const auto empty = httpGet("/residents", {{"q", ""}});
    const auto spaces = httpGet("/residents", {{"q", "   "}});

    CHECK(empty.status == 200);
    CHECK(contains(empty.body, "Reyes"));
    CHECK(contains(empty.body, "Cruz"));
    CHECK(spaces.status == 200);
    CHECK(contains(spaces.body, "Reyes"));
    CHECK(contains(spaces.body, "Cruz"));
}

DROGON_TEST(ResidentUiRegistrationFormShowsRequiredFields)
{
    TestAppFixture fixture("resident_ui_form_test.db");

    const auto reply = httpGet("/residents/new");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, R"(action="/residents")"));
    CHECK(contains(reply.body, R"(name="firstName")"));
    CHECK(contains(reply.body, R"(name="lastName")"));
    CHECK(contains(reply.body, R"(name="address")"));
    CHECK(contains(reply.body, R"(name="contactNumber")"));
    CHECK(contains(reply.body, R"(name="email")"));
}

DROGON_TEST(ResidentUiValidRegistrationPersistsAndRedirects)
{
    TestAppFixture fixture("resident_ui_register_valid_test.db");

    const auto reply = httpPostForm("/residents", {
        {"firstName", "Ana"},
        {"lastName", "Reyes"},
        {"address", "12 Mabini St"},
        {"contactNumber", "09171234567"},
        {"email", "ana@example.com"}});

    CHECK(reply.status == 303);
    CHECK(reply.location == "/residents?success=registered");

    const auto residents = fixture.context().residentQuery().listResidents();
    CHECK(residents.size() == 1);
    if (residents.size() == 1)
    {
        CHECK(residents[0].getFirstName() == "Ana");
        CHECK(residents[0].getLastName() == "Reyes");
        CHECK(residents[0].getContactNumber() == "09171234567");
        CHECK(residents[0].getStatus() == "Active");
    }

    const auto list = httpGet("/residents", {{"success", "registered"}});
    CHECK(contains(list.body, "Resident registered successfully."));
    CHECK(contains(list.body, "Reyes"));
}

DROGON_TEST(ResidentUiInvalidRegistrationShowsErrorsAndKeepsInput)
{
    TestAppFixture fixture("resident_ui_register_invalid_test.db");

    const auto reply = httpPostForm("/residents", {
        {"firstName", "\"><b>x"},
        {"lastName", ""},
        {"address", "12 Mabini St"},
        {"contactNumber", "123"},
        {"email", "bad"}});

    CHECK(reply.status == 400);
    CHECK(contains(reply.body, "Last name is required."));
    CHECK(contains(reply.body, "Contact number must be 11 digits starting with 09."));
    CHECK(contains(reply.body, "Enter a valid email address."));
    CHECK(!contains(reply.body, "First name is required."));
    // Entered values come back, escaped (no live markup from user input).
    CHECK(contains(reply.body, "&quot;&gt;&lt;b&gt;x"));
    CHECK(!contains(reply.body, "<b>x"));
    CHECK(contains(reply.body, "12 Mabini St"));
    // Nothing was saved.
    CHECK(fixture.context().residentQuery().listResidents().empty());
}

DROGON_TEST(ResidentUiEditFormShowsCurrentValuesAndReadOnlyStatus)
{
    TestAppFixture fixture("resident_ui_edit_form_test.db");
    const int id = registerResident(fixture, "Ana", "Reyes", "12 Mabini St",
                                    "09171234567", "ana@example.com");
    CHECK(id > 0);

    const auto reply = httpGet("/residents/" + std::to_string(id) + "/edit");

    CHECK(reply.status == 200);
    CHECK(contains(reply.body, R"(value="Ana")"));
    CHECK(contains(reply.body, R"(value="Reyes")"));
    CHECK(contains(reply.body, R"(value="12 Mabini St")"));
    CHECK(contains(reply.body, R"(value="09171234567")"));
    CHECK(contains(reply.body, R"(value="ana@example.com")"));
    CHECK(contains(reply.body, "Resident ID: " + std::to_string(id)));
    CHECK(contains(reply.body, "Status: Active"));
    // id and status are displayed, never editable inputs.
    CHECK(!contains(reply.body, R"(name="status")"));
    CHECK(!contains(reply.body, R"(name="id")"));
}

DROGON_TEST(ResidentUiValidUpdatePersistsAndKeepsStatus)
{
    TestAppFixture fixture("resident_ui_update_valid_test.db");
    const int id = registerResident(fixture, "Ana", "Reyes");
    CHECK(id > 0);

    // The forged "status" field must be ignored.
    const auto reply = httpPostForm("/residents/" + std::to_string(id), {
        {"firstName", "Ana"},
        {"lastName", "Reyes"},
        {"address", "99 Rizal Ave"},
        {"contactNumber", "09171234567"},
        {"email", "ana@example.com"},
        {"status", "Inactive"}});

    CHECK(reply.status == 303);
    CHECK(reply.location == "/residents?success=updated");

    const auto stored = fixture.context().residentQuery().findResident(id);
    CHECK(stored.has_value());
    if (stored)
    {
        CHECK(stored->getAddress() == "99 Rizal Ave");
        CHECK(stored->getStatus() == "Active");
    }

    const auto list = httpGet("/residents", {{"success", "updated"}});
    CHECK(contains(list.body, "Resident updated successfully."));
    CHECK(contains(list.body, "99 Rizal Ave"));
}

DROGON_TEST(ResidentUiInvalidUpdateShowsErrorsAndDoesNotModifyData)
{
    TestAppFixture fixture("resident_ui_update_invalid_test.db");
    const int id = registerResident(fixture, "Ana", "Reyes", "12 Mabini St");
    CHECK(id > 0);

    const auto reply = httpPostForm("/residents/" + std::to_string(id), {
        {"firstName", "Ana"},
        {"lastName", ""},
        {"address", "changed"},
        {"contactNumber", "09171234567"},
        {"email", "ana@example.com"}});

    CHECK(reply.status == 400);
    CHECK(contains(reply.body, "Last name is required."));
    CHECK(contains(reply.body, R"(value="changed")"));

    const auto stored = fixture.context().residentQuery().findResident(id);
    CHECK(stored.has_value());
    if (stored)
    {
        CHECK(stored->getLastName() == "Reyes");
        CHECK(stored->getAddress() == "12 Mabini St");
    }
}

DROGON_TEST(ResidentUiMissingOrMalformedResidentReturnsNotFound)
{
    TestAppFixture fixture("resident_ui_not_found_test.db");

    const auto missing = httpGet("/residents/99999/edit");
    CHECK(missing.status == 404);
    CHECK(contains(missing.body, "Resident not found."));

    const auto malformed = httpGet("/residents/abc/edit");
    CHECK(malformed.status == 404);

    const auto updateMissing = httpPostForm("/residents/99999", {
        {"firstName", "A"}, {"lastName", "B"}, {"address", "C"},
        {"contactNumber", "09171234567"}, {"email", "a@b.co"}});
    CHECK(updateMissing.status == 404);

    const auto deactivateMissing = httpPostForm("/residents/99999/deactivate");
    CHECK(deactivateMissing.status == 404);

    // Nothing was created by the rejected requests.
    CHECK(fixture.context().residentQuery().listResidents().empty());
}

DROGON_TEST(ResidentUiDeactivateMakesResidentInactive)
{
    TestAppFixture fixture("resident_ui_deactivate_test.db");
    const int id = registerResident(fixture, "Ana", "Reyes");
    CHECK(id > 0);

    const auto reply =
        httpPostForm("/residents/" + std::to_string(id) + "/deactivate");

    CHECK(reply.status == 303);
    CHECK(reply.location == "/residents?success=deactivated");

    const auto stored = fixture.context().residentQuery().findResident(id);
    CHECK(stored.has_value());
    if (stored)
    {
        CHECK(stored->getStatus() == "Inactive");
        CHECK(stored->getFirstName() == "Ana");
    }

    const auto list = httpGet("/residents", {{"success", "deactivated"}});
    CHECK(contains(list.body, "Resident deactivated successfully."));
}

DROGON_TEST(ResidentUiInactiveResidentRemainsVisibleWithoutDeactivateAction)
{
    TestAppFixture fixture("resident_ui_inactive_visible_test.db");
    const int id = registerResident(fixture, "Ana", "Reyes");
    CHECK(id > 0);
    CHECK(httpPostForm("/residents/" + std::to_string(id) + "/deactivate").status == 303);

    const auto list = httpGet("/residents");
    CHECK(list.status == 200);
    CHECK(contains(list.body, "Reyes"));
    CHECK(contains(list.body, "Inactive"));
    CHECK(!contains(list.body, "/residents/" + std::to_string(id) + "/deactivate"));

    const auto search = httpGet("/residents", {{"q", "Reyes"}});
    CHECK(contains(search.body, "Reyes"));
    CHECK(contains(search.body, "Inactive"));
}

DROGON_TEST(ResidentUiGetRequestsDoNotDeactivateResident)
{
    TestAppFixture fixture("resident_ui_get_readonly_test.db");
    const int id = registerResident(fixture, "Ana", "Reyes");
    CHECK(id > 0);
    const std::string idText = std::to_string(id);

    const auto getDeactivate = httpGet("/residents/" + idText + "/deactivate");
    CHECK(getDeactivate.status >= 400);

    CHECK(httpGet("/residents").status == 200);
    CHECK(httpGet("/residents/" + idText + "/edit").status == 200);
    CHECK(httpGet("/residents/new").status == 200);

    const auto stored = fixture.context().residentQuery().findResident(id);
    CHECK(stored.has_value());
    if (stored)
    {
        CHECK(stored->getStatus() == "Active");
    }
}