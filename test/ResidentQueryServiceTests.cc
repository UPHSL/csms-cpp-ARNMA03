#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../services/ResidentQueryService.h"

#include <cstdio>
#include <string>

// ---- T05 tests (Resident query service: blank-search handling) ----

DROGON_TEST(BlankSearchReturnsAllResidents)
{
    const std::string databasePath = "test_query_service_blank.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentQueryService queryService(repository);

    repository.save(Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com"));
    repository.save(Resident("Ben", "Torres", "Addr", "09172222222", "ben@example.com"));

    auto allResidents = queryService.listResidents();
    auto blankSearchResults = queryService.searchResidents("   ");

    CHECK(blankSearchResults.size() == allResidents.size());
    CHECK(blankSearchResults.size() == 2u);

    std::remove(databasePath.c_str());
}

DROGON_TEST(SearchTermWithSurroundingWhitespaceIsTrimmedBeforeSearching)
{
    const std::string databasePath = "test_query_service_trim.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentQueryService queryService(repository);

    repository.save(Resident("Juan", "Dela Cruz", "Addr", "09171111111", "juan@example.com"));

    auto results = queryService.searchResidents("  Juan  ");

    CHECK(results.size() == 1u);
    CHECK(results[0].getFirstName() == "Juan");

    std::remove(databasePath.c_str());
}

DROGON_TEST(EmptyStringSearchReturnsAllResidents)
{
    const std::string databasePath = "test_query_service_empty_string.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentQueryService queryService(repository);

    repository.save(Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com"));

    auto results = queryService.searchResidents("");

    CHECK(results.size() == 1u);

    std::remove(databasePath.c_str());
}