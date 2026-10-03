#include <drogon/drogon_test.h>
#include "../models/Resident.h"

// ---- T01 tests (updated to match corrected Resident model) ----

DROGON_TEST(ResidentCreation)
{
    Resident resident(
        "Juan",
        "Dela Cruz",
        "Laguna",
        "09123456789",
        "juan@example.com",
        "Active",
        1
    );

    CHECK(resident.getId().has_value());
    CHECK(resident.getId().value() == 1);
    CHECK(resident.getFirstName() == "Juan");
    CHECK(resident.getLastName() == "Dela Cruz");
}

DROGON_TEST(ResidentInformationAccess)
{
    Resident resident(
        "Maria",
        "Santos",
        "Binan, Laguna",
        "09987654321",
        "maria@example.com",
        "Active",
        2
    );

    CHECK(resident.getId().has_value());
    CHECK(resident.getId().value() == 2);
    CHECK(resident.getFirstName() == "Maria");
    CHECK(resident.getLastName() == "Santos");
    CHECK(resident.getAddress() == "Binan, Laguna");
    CHECK(resident.getContactNumber() == "09987654321");
    CHECK(resident.getEmail() == "maria@example.com");
}

DROGON_TEST(ResidentStatus)
{
    Resident resident(
        "Pedro",
        "Reyes",
        "Santa Rosa, Laguna",
        "09111222333",
        "pedro@example.com",
        "Active",
        3
    );

    CHECK(resident.getStatus() == "Active");
}

DROGON_TEST(ResidentDefaultsToActiveStatus)
{
    Resident resident(
        "Ana",
        "Cruz",
        "Calamba, Laguna",
        "09221234567",
        "ana@example.com"
    );

    CHECK(resident.getStatus() == "Active");
    CHECK(resident.getId().has_value() == false);
}