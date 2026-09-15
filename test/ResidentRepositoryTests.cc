#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"

#include <cstdio>
#include <string>

// ---- T03 tests (Resident persistence) ----

DROGON_TEST(PersistResident)
{
    const std::string databasePath = "test_persist_resident.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    Resident resident(
        "Arden",
        "Austria",
        "Barangay Santo Nino",
        "09676767676",
        "arden@example.com"
    );

    Resident savedResident = repository.save(resident);

    CHECK(savedResident.getId().has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(ResidentGetsGeneratedId)
{
    const std::string databasePath = "test_generated_id.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    Resident resident(
        "Roland",
        "Santos",
        "Street 123",
        "09181234567",
        "roland@example.com"
    );

    CHECK(!resident.getId().has_value());

    Resident savedResident = repository.save(resident);

    CHECK(savedResident.getId().has_value());
    CHECK(savedResident.getId().value() > 0);

    std::remove(databasePath.c_str());
}

DROGON_TEST(RetrieveResidentById)
{
    const std::string databasePath = "test_find_resident.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    Resident resident(
        "Pedro",
        "Garcia",
        "789 Laguna Street",
        "09191234567",
        "pedro@example.com"
    );

    Resident savedResident = repository.save(resident);

    int residentId = savedResident.getId().value();

    auto foundResident = repository.findById(residentId);

    CHECK(foundResident.has_value());
    CHECK(foundResident->getId().has_value());
    CHECK(foundResident->getId().value() == residentId);

    std::remove(databasePath.c_str());
}

DROGON_TEST(ResidentInformationIsPreserved)
{
    const std::string databasePath = "test_information_preserved.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    Resident resident(
        "Juan",
        "Dela Cruz",
        "123 Main Street, Laguna",
        "09171234567",
        "juan@example.com"
    );

    Resident savedResident = repository.save(resident);

    int residentId = savedResident.getId().value();

    auto foundResident = repository.findById(residentId);

    CHECK(foundResident.has_value());

    CHECK(foundResident->getFirstName() == "Juan");
    CHECK(foundResident->getLastName() == "Dela Cruz");
    CHECK(foundResident->getAddress() == "123 Main Street, Laguna");
    CHECK(foundResident->getContactNumber() == "09171234567");
    CHECK(foundResident->getEmail() == "juan@example.com");
    CHECK(foundResident->getStatus() == "Active");

    std::remove(databasePath.c_str());
}

DROGON_TEST(ResidentActiveStatusIsPreserved)
{
    const std::string databasePath = "test_active_status.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    Resident resident(
        "Ana",
        "Reyes",
        "456 Bayani Road",
        "09201234567",
        "ana@example.com",
        "Active"
    );

    Resident savedResident = repository.save(resident);

    int residentId = savedResident.getId().value();

    auto foundResident = repository.findById(residentId);

    CHECK(foundResident.has_value());
    CHECK(foundResident->getStatus() == "Active");

    std::remove(databasePath.c_str());
}

DROGON_TEST(MissingResidentReturnsNullopt)
{
    const std::string databasePath = "test_missing_resident.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    auto foundResident = repository.findById(9999);

    CHECK(!foundResident.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(NewRepositoryCanRetrieveResident)
{
    const std::string databasePath = "test_repository_persistence.db";

    std::remove(databasePath.c_str());

    int residentId = 0;

    {
        Database database(databasePath);
        database.initialize();

        ResidentRepository repository(database);

        Resident resident(
            "Carlos",
            "Mendoza",
            "789 Mabini Street",
            "09301234567",
            "carlos@example.com"
        );

        Resident savedResident = repository.save(resident);

        CHECK(savedResident.getId().has_value());

        residentId = savedResident.getId().value();
    }

    {
        Database database(databasePath);
        database.initialize();

        ResidentRepository repository(database);

        auto foundResident = repository.findById(residentId);

        CHECK(foundResident.has_value());
        CHECK(foundResident->getId().value() == residentId);
        CHECK(foundResident->getFirstName() == "Carlos");
        CHECK(foundResident->getLastName() == "Mendoza");
    }

    std::remove(databasePath.c_str());
}

DROGON_TEST(MultipleResidentsReceiveUniqueIds)
{
    const std::string databasePath = "test_unique_ids.db";

    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    Resident firstResident(
        "Maria",
        "Santos",
        "101 Laguna Street",
        "09171234567",
        "maria@example.com"
    );

    Resident secondResident(
        "Jose",
        "Cruz",
        "202 Laguna Street",
        "09281234567",
        "jose@example.com"
    );

    Resident savedFirst = repository.save(firstResident);
    Resident savedSecond = repository.save(secondResident);

    CHECK(savedFirst.getId().has_value());
    CHECK(savedSecond.getId().has_value());

    CHECK(savedFirst.getId().value() != savedSecond.getId().value());

    std::remove(databasePath.c_str());
}