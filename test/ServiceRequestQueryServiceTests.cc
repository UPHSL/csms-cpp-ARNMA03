#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../models/ServiceRequest.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"
#include "../services/ServiceRequestQueryService.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

// ---- T11 tests (Search and View Service Requests) ----

namespace
{
    // Owns a fresh database plus everything a T11 test needs.
    struct QueryFixture
    {
        Database database;
        ResidentRepository residentRepository;
        ServiceRequestRepository serviceRequestRepository;
        ServiceRequestQueryService service;
        int residentId;

        explicit QueryFixture(const std::string& databasePath)
            : database(freshPath(databasePath)),
              residentRepository(database),
              serviceRequestRepository(database),
              service(serviceRequestRepository),
              residentId(0)
        {
            database.initialize();

            residentId = residentRepository.save(
                Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active")
            ).getId().value();
        }

        static const std::string& freshPath(const std::string& path)
        {
            std::remove(path.c_str());
            return path;
        }
    };

    // Persists a request directly through the repository so any status can be
    // set up without walking the T10 workflow. Returns the generated id.
    int addRequest(
        QueryFixture& fixture,
        const std::string& serviceType,
        const std::string& description,
        const std::string& dateRequested,
        const std::string& status = "Pending"
    )
    {
        ServiceRequest saved = fixture.serviceRequestRepository.save(
            ServiceRequest(
                fixture.residentId,
                serviceType,
                description,
                dateRequested,
                status));

        return saved.getId().value();
    }

    std::vector<int> idsOf(const std::vector<ServiceRequest>& requests)
    {
        std::vector<int> ids;

        for (const auto& request : requests)
        {
            ids.push_back(request.getId().value());
        }

        return ids;
    }
}

// Test 1 - List all persisted Service Requests
DROGON_TEST(ServiceRequestQueryListsAllPersistedRequests)
{
    const std::string databasePath = "test_sr_query_list_all.db";
    QueryFixture fixture(databasePath);

    int first = addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15");
    int second = addRequest(fixture, "Certificate Request", "Residency certificate", "2026-09-16");
    int third = addRequest(fixture, "Permit Request", "Business permit", "2026-09-17");

    auto requests = fixture.service.listServiceRequests();
    auto ids = idsOf(requests);

    CHECK(requests.size() == 3u);
    CHECK(std::find(ids.begin(), ids.end(), first) != ids.end());
    CHECK(std::find(ids.begin(), ids.end(), second) != ids.end());
    CHECK(std::find(ids.begin(), ids.end(), third) != ids.end());

    std::remove(databasePath.c_str());
}

// Test 2 - Empty listing returns an empty collection
DROGON_TEST(ServiceRequestQueryEmptyListingReturnsEmptyCollection)
{
    const std::string databasePath = "test_sr_query_empty.db";
    QueryFixture fixture(databasePath);

    auto requests = fixture.service.listServiceRequests();

    CHECK(requests.empty());

    std::remove(databasePath.c_str());
}

// Test 3 - Listing uses dateRequested DESC, then id DESC
DROGON_TEST(ServiceRequestQueryListingUsesRequiredOrdering)
{
    const std::string databasePath = "test_sr_query_ordering.db";
    QueryFixture fixture(databasePath);

    // Inserted deliberately out of the required order.
    int a = addRequest(fixture, "Type A", "Description A", "2026-09-15");
    int b = addRequest(fixture, "Type B", "Description B", "2026-09-18");
    int c = addRequest(fixture, "Type C", "Description C", "2026-09-10");
    int d = addRequest(fixture, "Type D", "Description D", "2026-09-18"); // same date as b, higher id

    auto ids = idsOf(fixture.service.listServiceRequests());

    // 2026-09-18 (higher id first), then 2026-09-15, then 2026-09-10.
    CHECK(ids.size() == 4u);
    CHECK(ids == std::vector<int>({d, b, a, c}));

    std::remove(databasePath.c_str());
}

// Test 4 - View an existing Service Request by ID
DROGON_TEST(ServiceRequestQueryViewsExistingRequestById)
{
    const std::string databasePath = "test_sr_query_view.db";
    QueryFixture fixture(databasePath);

    addRequest(fixture, "Permit Request", "Business permit", "2026-09-17");
    int id = addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto found = fixture.service.viewServiceRequest(id);

    CHECK(found.has_value());
    CHECK(found->getId().value() == id);
    CHECK(found->getServiceType() == "Barangay Clearance");

    std::remove(databasePath.c_str());
}

// Test 5 - Viewing an unknown ID is handled safely and creates nothing
DROGON_TEST(ServiceRequestQueryViewingUnknownIdIsHandledSafely)
{
    const std::string databasePath = "test_sr_query_view_unknown.db";
    QueryFixture fixture(databasePath);

    int existingId = addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto found = fixture.service.viewServiceRequest(9999);

    CHECK(!found.has_value());

    // No placeholder record was created and the existing one is untouched.
    CHECK(fixture.service.listServiceRequests().size() == 1u);
    CHECK(!fixture.serviceRequestRepository.findById(9999).has_value());
    CHECK(fixture.serviceRequestRepository.findById(existingId)->getStatus() == "Pending");

    std::remove(databasePath.c_str());
}

// Test 6 - Partial service-type search is case-insensitive
DROGON_TEST(ServiceRequestQueryPartialServiceTypeSearchIsCaseInsensitive)
{
    const std::string databasePath = "test_sr_query_type_search.db";
    QueryFixture fixture(databasePath);

    int clearanceId = addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15");
    addRequest(fixture, "Certificate Request", "Residency certificate", "2026-09-16");

    auto result = fixture.service.searchServiceRequests("cLeAr");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.size() == 1u);
    CHECK(result.serviceRequests[0].getId().value() == clearanceId);

    std::remove(databasePath.c_str());
}

// Test 7 - Partial description search is case-insensitive
DROGON_TEST(ServiceRequestQueryPartialDescriptionSearchIsCaseInsensitive)
{
    const std::string databasePath = "test_sr_query_description_search.db";
    QueryFixture fixture(databasePath);

    int matchId = addRequest(
        fixture, "Barangay Clearance",
        "Requesting barangay clearance for employment requirements.", "2026-09-15");
    addRequest(fixture, "Certificate Request", "Residency certificate", "2026-09-16");

    auto result = fixture.service.searchServiceRequests("EmPlOy");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.size() == 1u);
    CHECK(result.serviceRequests[0].getId().value() == matchId);

    std::remove(databasePath.c_str());
}

// Test 8 - Blank search returns the same requests, in the same order, as listing
DROGON_TEST(ServiceRequestQueryBlankSearchReturnsAllInRequiredOrder)
{
    const std::string databasePath = "test_sr_query_blank.db";
    QueryFixture fixture(databasePath);

    addRequest(fixture, "Type A", "Description A", "2026-09-15", "Pending");
    addRequest(fixture, "Type B", "Description B", "2026-09-18", "Completed");
    addRequest(fixture, "Type C", "Description C", "2026-09-10", "Cancelled");
    addRequest(fixture, "Type D", "Description D", "2026-09-18", "In Progress");

    auto listed = idsOf(fixture.service.listServiceRequests());

    auto blank = fixture.service.searchServiceRequests("      ");
    auto empty = fixture.service.searchServiceRequests("");

    CHECK(blank.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(listed.size() == 4u);
    CHECK(idsOf(blank.serviceRequests) == listed);
    CHECK(idsOf(empty.serviceRequests) == listed);

    std::remove(databasePath.c_str());
}

// Test 9 - No text match returns an empty collection
DROGON_TEST(ServiceRequestQueryNoTextMatchReturnsEmptyCollection)
{
    const std::string databasePath = "test_sr_query_no_match.db";
    QueryFixture fixture(databasePath);

    addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto result = fixture.service.searchServiceRequests("ZzzNoSuchText");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.empty());

    std::remove(databasePath.c_str());
}

// Test 10 - Status filter returns only the matching status
DROGON_TEST(ServiceRequestQueryStatusFilterReturnsOnlyMatchingStatus)
{
    const std::string databasePath = "test_sr_query_status_filter.db";
    QueryFixture fixture(databasePath);

    addRequest(fixture, "Type A", "Description A", "2026-09-15", "Pending");
    addRequest(fixture, "Type B", "Description B", "2026-09-16", "Completed");
    addRequest(fixture, "Type C", "Description C", "2026-09-17", "Pending");
    addRequest(fixture, "Type D", "Description D", "2026-09-18", "Cancelled");

    auto result = fixture.service.searchServiceRequests("", "Pending");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.size() == 2u);

    for (const auto& request : result.serviceRequests)
    {
        CHECK(request.getStatus() == "Pending");
    }

    std::remove(databasePath.c_str());
}

// Test 11 - Unfiltered query includes every lifecycle status
DROGON_TEST(ServiceRequestQueryUnfilteredIncludesAllStatuses)
{
    const std::string databasePath = "test_sr_query_all_statuses.db";
    QueryFixture fixture(databasePath);

    addRequest(fixture, "Type A", "Description A", "2026-09-15", "Pending");
    addRequest(fixture, "Type B", "Description B", "2026-09-16", "In Progress");
    addRequest(fixture, "Type C", "Description C", "2026-09-17", "Completed");
    addRequest(fixture, "Type D", "Description D", "2026-09-18", "Cancelled");

    auto requests = fixture.service.listServiceRequests();

    bool hasPending = false;
    bool hasInProgress = false;
    bool hasCompleted = false;
    bool hasCancelled = false;

    for (const auto& request : requests)
    {
        if (request.getStatus() == "Pending") hasPending = true;
        if (request.getStatus() == "In Progress") hasInProgress = true;
        if (request.getStatus() == "Completed") hasCompleted = true;
        if (request.getStatus() == "Cancelled") hasCancelled = true;
    }

    CHECK(requests.size() == 4u);
    CHECK(hasPending);
    CHECK(hasInProgress);
    CHECK(hasCompleted);
    CHECK(hasCancelled);

    std::remove(databasePath.c_str());
}

// Test 12 - Text search AND status filter combined
DROGON_TEST(ServiceRequestQueryCombinesTextSearchAndStatusFilter)
{
    const std::string databasePath = "test_sr_query_combined.db";
    QueryFixture fixture(databasePath);

    // Matches text and status.
    int bothId = addRequest(
        fixture, "Barangay Clearance", "Employment requirement", "2026-09-15", "Pending");
    // Matches text only.
    addRequest(
        fixture, "Barangay Clearance", "Scholarship requirement", "2026-09-16", "Completed");
    // Matches status only.
    addRequest(
        fixture, "Community Assistance", "Employment support", "2026-09-17", "Pending");
    // Matches neither.
    addRequest(
        fixture, "Community Assistance", "Medical support", "2026-09-18", "Completed");

    auto result = fixture.service.searchServiceRequests("clearance", "Pending");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.size() == 1u);
    CHECK(result.serviceRequests[0].getId().value() == bothId);

    std::remove(databasePath.c_str());
}

// Test 13 - A request matching in both searchable fields appears once
DROGON_TEST(ServiceRequestQueryMatchingBothFieldsIsNotDuplicated)
{
    const std::string databasePath = "test_sr_query_no_duplicate.db";
    QueryFixture fixture(databasePath);

    addRequest(
        fixture,
        "Community Assistance",
        "Assistance requested for medical support",
        "2026-09-15");

    auto result = fixture.service.searchServiceRequests("assistance");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.size() == 1u);

    std::remove(databasePath.c_str());
}

// Test 14 - Query results preserve complete Service Request information
DROGON_TEST(ServiceRequestQueryResultsPreserveCompleteInformation)
{
    const std::string databasePath = "test_sr_query_preserved.db";
    QueryFixture fixture(databasePath);

    int id = addRequest(
        fixture,
        "Barangay Clearance",
        "Requesting barangay clearance for employment requirements.",
        "2026-09-15",
        "Completed");

    auto listed = fixture.service.listServiceRequests();
    auto searched = fixture.service.searchServiceRequests("clearance", "Completed");
    auto viewed = fixture.service.viewServiceRequest(id);

    CHECK(listed.size() == 1u);
    CHECK(searched.serviceRequests.size() == 1u);
    CHECK(viewed.has_value());

    const ServiceRequest results[] = { listed[0], searched.serviceRequests[0], *viewed };

    for (const auto& request : results)
    {
        CHECK(request.getId().value() == id);
        CHECK(request.getResidentId() == fixture.residentId);
        CHECK(request.getServiceType() == "Barangay Clearance");
        CHECK(request.getDescription() == "Requesting barangay clearance for employment requirements.");
        CHECK(request.getDateRequested() == "2026-09-15");
        CHECK(request.getStatus() == "Completed");
    }

    std::remove(databasePath.c_str());
}

// Extra - Leading and trailing whitespace in the search term is ignored
DROGON_TEST(ServiceRequestQuerySearchTermWhitespaceIsTrimmed)
{
    const std::string databasePath = "test_sr_query_trim.db";
    QueryFixture fixture(databasePath);

    int id = addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15");

    auto result = fixture.service.searchServiceRequests("   clearance   ");

    CHECK(result.outcome == ServiceRequestSearchOutcome::Success);
    CHECK(result.serviceRequests.size() == 1u);
    CHECK(result.serviceRequests[0].getId().value() == id);

    // Stored information is not modified by the normalization.
    CHECK(fixture.serviceRequestRepository.findById(id)->getServiceType() == "Barangay Clearance");

    std::remove(databasePath.c_str());
}

// Extra - Unsupported status filters are not silently accepted
DROGON_TEST(ServiceRequestQueryUnsupportedStatusFilterIsRejected)
{
    const std::string databasePath = "test_sr_query_bad_status.db";
    QueryFixture fixture(databasePath);

    addRequest(fixture, "Barangay Clearance", "Employment requirement", "2026-09-15", "Pending");

    for (const std::string badStatus : {"Approved", "Done", "Closed", "pending"})
    {
        auto result = fixture.service.searchServiceRequests("", badStatus);

        CHECK(result.outcome == ServiceRequestSearchOutcome::UnsupportedStatusFilter);
        CHECK(result.serviceRequests.empty());
    }

    // Persistence is untouched.
    CHECK(fixture.service.listServiceRequests().size() == 1u);

    std::remove(databasePath.c_str());
}
