#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../services/ResidentDeactivationService.h"
#include "../services/ResidentQueryService.h"

#include <cstdio>
#include <string>

// ---- T07 tests (Deactivate a Resident) ----

DROGON_TEST(ActiveResidentCanBeDeactivated)
{
    const std::string databasePath = "test_deactivate_active.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));

    auto result = service.deactivate(saved.getId().value());

    CHECK(result.outcome == ResidentDeactivationOutcome::Deactivated);
    CHECK(result.resident.has_value());
    CHECK(result.resident->getStatus() == "Inactive");

    std::remove(databasePath.c_str());
}

DROGON_TEST(StatusBecomesInactiveInPersistence)
{
    const std::string databasePath = "test_deactivate_persisted.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));
    int id = saved.getId().value();

    service.deactivate(id);

    auto found = repository.findById(id);
    CHECK(found.has_value());
    CHECK(found->getStatus() == "Inactive");

    std::remove(databasePath.c_str());
}

DROGON_TEST(DeactivationPreservesId)
{
    const std::string databasePath = "test_deactivate_id.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));
    int id = saved.getId().value();

    auto result = service.deactivate(id);

    CHECK(result.resident->getId().value() == id);

    std::remove(databasePath.c_str());
}

DROGON_TEST(DeactivationPreservesPersonalAndContactInformation)
{
    const std::string databasePath = "test_deactivate_info_preserved.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));
    int id = saved.getId().value();

    service.deactivate(id);

    auto found = repository.findById(id);
    CHECK(found->getFirstName() == "Juan");
    CHECK(found->getLastName() == "Cruz");
    CHECK(found->getAddress() == "Barangay Santo Tomas");
    CHECK(found->getContactNumber() == "09171234567");
    CHECK(found->getEmail() == "juan@example.com");

    std::remove(databasePath.c_str());
}

DROGON_TEST(DeactivatedResidentRemainsPersistedAndRetrievable)
{
    const std::string databasePath = "test_deactivate_not_deleted.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));
    int id = saved.getId().value();

    service.deactivate(id);

    // Must still be found -- this is a soft deactivation, not a delete.
    auto found = repository.findById(id);
    CHECK(found.has_value());
    CHECK(repository.findAll().size() == 1u);

    std::remove(databasePath.c_str());
}

DROGON_TEST(DeactivatedResidentRemainsVisibleThroughT05)
{
    const std::string databasePath = "test_deactivate_t05_visible.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService deactivationService(repository);
    ResidentQueryService queryService(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Active"));
    int id = saved.getId().value();

    deactivationService.deactivate(id);

    auto listed = queryService.listResidents();
    CHECK(listed.size() == 1u);

    auto searched = queryService.searchResidents("Cruz");
    CHECK(searched.size() == 1u);
    CHECK(searched[0].getStatus() == "Inactive");

    std::remove(databasePath.c_str());
}

DROGON_TEST(AlreadyInactiveResidentIsHandledSafely)
{
    const std::string databasePath = "test_deactivate_already_inactive.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Barangay Santo Tomas", "09171234567", "juan@example.com", "Inactive"));
    int id = saved.getId().value();

    auto result = service.deactivate(id);

    CHECK(result.outcome == ResidentDeactivationOutcome::AlreadyInactive);
    CHECK(result.resident.has_value());
    CHECK(result.resident->getStatus() == "Inactive");

    // Calling it again should not crash and should not change the ID or info.
    auto secondResult = service.deactivate(id);
    CHECK(secondResult.outcome == ResidentDeactivationOutcome::AlreadyInactive);
    CHECK(secondResult.resident->getId().value() == id);
    CHECK(secondResult.resident->getFirstName() == "Juan");

    CHECK(repository.findAll().size() == 1u);

    std::remove(databasePath.c_str());
}

DROGON_TEST(NonexistentResidentDeactivationReturnsNotFound)
{
    const std::string databasePath = "test_deactivate_not_found.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    auto result = service.deactivate(9999);

    CHECK(result.outcome == ResidentDeactivationOutcome::NotFound);
    CHECK(!result.resident.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(NonexistentDeactivationDoesNotCreateOrDeleteRecords)
{
    const std::string databasePath = "test_deactivate_not_found_no_side_effects.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    repository.save(
        Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com", "Active"));

    service.deactivate(9999);

    CHECK(repository.findAll().size() == 1u);
    CHECK(!repository.findById(9999).has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(DeactivatingOneResidentDoesNotAffectAnother)
{
    const std::string databasePath = "test_deactivate_isolated.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentRepository repository(database);
    ResidentDeactivationService service(repository);

    Resident first = repository.save(
        Resident("Juan", "Cruz", "Addr 1", "09171111111", "juan@example.com", "Active"));
    Resident second = repository.save(
        Resident("Ana", "Reyes", "Addr 2", "09172222222", "ana@example.com", "Active"));

    service.deactivate(first.getId().value());

    auto foundFirst = repository.findById(first.getId().value());
    auto foundSecond = repository.findById(second.getId().value());

    CHECK(foundFirst->getStatus() == "Inactive");
    CHECK(foundSecond->getStatus() == "Active");

    std::remove(databasePath.c_str());
}
