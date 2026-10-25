#include <drogon/drogon_test.h>
#include "../models/ServiceRequest.h"

// ---- T08 tests (Service Request domain model) ----

DROGON_TEST(ServiceRequestCanBeCreated)
{
    ServiceRequest request(
        25,
        "Barangay Clearance",
        "Request for employment requirement",
        "2026-09-15"
    );

    // No crash / successful construction is the assertion here.
    CHECK(request.getResidentId() == 25);
}

DROGON_TEST(ServiceRequestInformationIsAccessible)
{
    ServiceRequest request(
        25,
        "Barangay Clearance",
        "Request for employment requirement",
        "2026-09-15"
    );

    CHECK(request.getResidentId() == 25);
    CHECK(request.getServiceType() == "Barangay Clearance");
    CHECK(request.getDescription() == "Request for employment requirement");
    CHECK(request.getDateRequested() == "2026-09-15");
}

DROGON_TEST(ResidentIdIsPreserved)
{
    ServiceRequest request(
        25,
        "Certificate Request",
        "Requesting a certificate of residency",
        "2026-09-16"
    );

    CHECK(request.getResidentId() == 25);
}

DROGON_TEST(NewServiceRequestHasUnassignedId)
{
    ServiceRequest request(
        25,
        "Community Assistance",
        "Requesting financial assistance",
        "2026-09-17"
    );

    CHECK(!request.getId().has_value());
}

DROGON_TEST(NewServiceRequestDefaultsToPending)
{
    ServiceRequest request(
        25,
        "Permit Request",
        "Requesting a business permit",
        "2026-09-18"
    );

    CHECK(request.getStatus() == "Pending");
}

DROGON_TEST(ServiceRequestInformationIsIndependentBetweenObjects)
{
    ServiceRequest first(
        25,
        "Barangay Clearance",
        "First request description",
        "2026-09-15"
    );

    ServiceRequest second(
        30,
        "Certificate Request",
        "Second request description",
        "2026-09-20"
    );

    CHECK(first.getResidentId() == 25);
    CHECK(first.getServiceType() == "Barangay Clearance");
    CHECK(first.getDescription() == "First request description");
    CHECK(first.getDateRequested() == "2026-09-15");

    CHECK(second.getResidentId() == 30);
    CHECK(second.getServiceType() == "Certificate Request");
    CHECK(second.getDescription() == "Second request description");
    CHECK(second.getDateRequested() == "2026-09-20");
}
