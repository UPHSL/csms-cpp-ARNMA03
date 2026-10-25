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

// ---- T05 tests (Resident search and listing) ----

DROGON_TEST(FindAllReturnsAllPersistedResidents)
{
    const std::string databasePath = "test_find_all.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Ana", "Reyes", "Addr 1", "09171111111", "ana@example.com"));
    repository.save(Resident("Ben", "Torres", "Addr 2", "09172222222", "ben@example.com"));
    repository.save(Resident("Cathy", "Lim", "Addr 3", "09173333333", "cathy@example.com"));

    auto residents = repository.findAll();

    CHECK(residents.size() == 3u);

    std::remove(databasePath.c_str());
}

DROGON_TEST(FindAllOnEmptyDatabaseReturnsEmptyCollection)
{
    const std::string databasePath = "test_find_all_empty.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    auto residents = repository.findAll();

    CHECK(residents.empty());

    std::remove(databasePath.c_str());
}

DROGON_TEST(FindAllOrdersByLastNameThenFirstNameThenId)
{
    const std::string databasePath = "test_find_all_ordering.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Ana", "Santos", "Addr", "09171111111", "ana@example.com"));
    repository.save(Resident("Pedro", "Cruz", "Addr", "09172222222", "pedro@example.com"));
    repository.save(Resident("Maria", "Andres", "Addr", "09173333333", "maria@example.com"));
    repository.save(Resident("Juan", "Cruz", "Addr", "09174444444", "juan@example.com"));

    auto residents = repository.findAll();

    CHECK(residents.size() == 4u);
    CHECK(residents[0].getLastName() == "Andres");
    CHECK(residents[1].getLastName() == "Cruz");
    CHECK(residents[1].getFirstName() == "Juan");
    CHECK(residents[2].getLastName() == "Cruz");
    CHECK(residents[2].getFirstName() == "Pedro");
    CHECK(residents[3].getLastName() == "Santos");

    std::remove(databasePath.c_str());
}

DROGON_TEST(SearchByNameMatchesPartialFirstNameCaseInsensitively)
{
    const std::string databasePath = "test_search_first_name.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Juan", "Bautista", "Addr", "09171111111", "juan@example.com"));

    auto results = repository.searchByName("jUa");

    CHECK(results.size() == 1u);
    CHECK(results[0].getFirstName() == "Juan");

    std::remove(databasePath.c_str());
}

DROGON_TEST(SearchByNameMatchesPartialLastNameCaseInsensitively)
{
    const std::string databasePath = "test_search_last_name.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Juan", "Dela Cruz", "Addr", "09171111111", "juan@example.com"));

    auto results = repository.searchByName("cRuZ");

    CHECK(results.size() == 1u);
    CHECK(results[0].getLastName() == "Dela Cruz");

    std::remove(databasePath.c_str());
}

DROGON_TEST(SearchByNameWithNoMatchReturnsEmptyCollection)
{
    const std::string databasePath = "test_search_no_match.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Juan", "Dela Cruz", "Addr", "09171111111", "juan@example.com"));

    auto results = repository.searchByName("ZzzUnknownResident");

    CHECK(results.empty());

    std::remove(databasePath.c_str());
}

DROGON_TEST(SearchByNamePreservesResidentInformation)
{
    const std::string databasePath = "test_search_preserved.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident(
        "Miguel",
        "Aquino",
        "123 Rizal Street",
        "09201234567",
        "miguel@example.com",
        "Inactive"
    ));

    auto results = repository.searchByName("Miguel");

    CHECK(results.size() == 1u);
    CHECK(results[0].getId().has_value());
    CHECK(results[0].getFirstName() == "Miguel");
    CHECK(results[0].getLastName() == "Aquino");
    CHECK(results[0].getAddress() == "123 Rizal Street");
    CHECK(results[0].getContactNumber() == "09201234567");
    CHECK(results[0].getEmail() == "miguel@example.com");
    CHECK(results[0].getStatus() == "Inactive");

    std::remove(databasePath.c_str());
}

DROGON_TEST(FindAllIncludesActiveAndInactiveResidents)
{
    const std::string databasePath = "test_find_all_statuses.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com", "Active"));
    repository.save(Resident("Ben", "Torres", "Addr", "09172222222", "ben@example.com", "Inactive"));

    auto residents = repository.findAll();

    CHECK(residents.size() == 2u);

    bool hasActive = false;
    bool hasInactive = false;

    for (const auto& resident : residents)
    {
        if (resident.getStatus() == "Active") hasActive = true;
        if (resident.getStatus() == "Inactive") hasInactive = true;
    }

    CHECK(hasActive);
    CHECK(hasInactive);

    std::remove(databasePath.c_str());
}

DROGON_TEST(SearchByNameDoesNotDuplicateResidentMatchingBothNames)
{
    const std::string databasePath = "test_search_no_duplicate.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);

    repository.save(Resident("Cruz", "Cruzana", "Addr", "09171111111", "cruz@example.com"));

    auto results = repository.searchByName("Cruz");

    CHECK(results.size() == 1u);

    std::remove(databasePath.c_str());
}