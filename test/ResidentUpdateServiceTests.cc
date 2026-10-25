#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../services/ResidentUpdateService.h"
#include "../services/ResidentQueryService.h"

#include <cstdio>
#include <string>

// ---- T06 tests (Update Resident information) ----

namespace
{
    ResidentUpdateInformation validInformation()
    {
        return ResidentUpdateInformation{
            "Juan Miguel",
            "Dela Cruz",
            "456 New Street, Laguna",
            "09189876543",
            "juanmiguel@example.com"
        };
    }
}

DROGON_TEST(ValidUpdateSucceeds)
{
    const std::string databasePath = "test_update_valid.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));

    auto result = service.updateResident(saved.getId().value(), validInformation());

    CHECK(result.success == true);
    CHECK(result.notFound == false);
    CHECK(result.resident.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(UpdatePreservesResidentId)
{
    const std::string databasePath = "test_update_id.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));
    int id = saved.getId().value();

    auto result = service.updateResident(id, validInformation());

    CHECK(result.resident->getId().value() == id);
    CHECK(repository.findAll().size() == 1u);

    std::remove(databasePath.c_str());
}

DROGON_TEST(UpdatePersistsAllEditableFields)
{
    const std::string databasePath = "test_update_fields.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));
    int id = saved.getId().value();

    service.updateResident(id, validInformation());

    auto found = repository.findById(id);

    CHECK(found.has_value());
    CHECK(found->getFirstName() == "Juan Miguel");
    CHECK(found->getLastName() == "Dela Cruz");
    CHECK(found->getAddress() == "456 New Street, Laguna");
    CHECK(found->getContactNumber() == "09189876543");
    CHECK(found->getEmail() == "juanmiguel@example.com");

    std::remove(databasePath.c_str());
}

DROGON_TEST(UpdatePreservesActiveStatus)
{
    const std::string databasePath = "test_update_status_active.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com", "Active"));
    int id = saved.getId().value();

    service.updateResident(id, validInformation());

    CHECK(repository.findById(id)->getStatus() == "Active");

    std::remove(databasePath.c_str());
}

DROGON_TEST(UpdatePreservesInactiveStatus)
{
    const std::string databasePath = "test_update_status_inactive.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com", "Inactive"));
    int id = saved.getId().value();

    service.updateResident(id, validInformation());

    CHECK(repository.findById(id)->getStatus() == "Inactive");

    std::remove(databasePath.c_str());
}

DROGON_TEST(InvalidUpdateFailsWithValidationDetails)
{
    const std::string databasePath = "test_update_invalid.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));
    int id = saved.getId().value();

    ResidentUpdateInformation blankFirst = validInformation();
    blankFirst.firstName = "";
    auto r1 = service.updateResident(id, blankFirst);
    CHECK(r1.success == false);
    CHECK(r1.notFound == false);
    CHECK(r1.validationResult.firstNameValid == false);

    ResidentUpdateInformation blankLast = validInformation();
    blankLast.lastName = "";
    CHECK(service.updateResident(id, blankLast).validationResult.lastNameValid == false);

    ResidentUpdateInformation blankAddress = validInformation();
    blankAddress.address = "";
    CHECK(service.updateResident(id, blankAddress).validationResult.addressValid == false);

    ResidentUpdateInformation badPhone = validInformation();
    badPhone.contactNumber = "0917ABC4567";
    CHECK(service.updateResident(id, badPhone).validationResult.contactNumberValid == false);

    ResidentUpdateInformation badEmail = validInformation();
    badEmail.email = "juan.example.com";
    CHECK(service.updateResident(id, badEmail).validationResult.emailValid == false);

    std::remove(databasePath.c_str());
}

DROGON_TEST(InvalidUpdateDoesNotModifyExistingData)
{
    const std::string databasePath = "test_update_invalid_unchanged.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));
    int id = saved.getId().value();

    ResidentUpdateInformation invalid = validInformation();
    invalid.email = "not-an-email";

    auto result = service.updateResident(id, invalid);

    CHECK(result.success == false);
    CHECK(!result.resident.has_value());

    auto found = repository.findById(id);
    CHECK(found->getFirstName() == "Juan");
    CHECK(found->getLastName() == "Cruz");
    CHECK(found->getAddress() == "Old Address");
    CHECK(found->getContactNumber() == "09171234567");
    CHECK(found->getEmail() == "juan@example.com");

    std::remove(databasePath.c_str());
}

DROGON_TEST(NonexistentResidentReturnsNotFound)
{
    const std::string databasePath = "test_update_not_found.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    auto result = service.updateResident(9999, validInformation());

    CHECK(result.success == false);
    CHECK(result.notFound == true);
    CHECK(!result.resident.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(NonexistentUpdateDoesNotCreateResident)
{
    const std::string databasePath = "test_update_not_found_no_create.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    repository.save(
        Resident("Ana", "Reyes", "Addr", "09171111111", "ana@example.com"));

    service.updateResident(9999, validInformation());

    CHECK(repository.findAll().size() == 1u);
    CHECK(!repository.findById(9999).has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(UpdatedResidentIsFoundBySearch)
{
    const std::string databasePath = "test_update_search.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService updateService(validator, repository);
    ResidentQueryService queryService(repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));
    int id = saved.getId().value();

    updateService.updateResident(id, validInformation());

    auto byNewName = queryService.searchResidents("Miguel");
    CHECK(byNewName.size() == 1u);
    CHECK(byNewName[0].getId().value() == id);


    std::remove(databasePath.c_str());
}

DROGON_TEST(UpdatedContactNumberKeepsLeadingZero)
{
    const std::string databasePath = "test_update_leading_zero.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentUpdateService service(validator, repository);

    Resident saved = repository.save(
        Resident("Juan", "Cruz", "Old Address", "09171234567", "juan@example.com"));
    int id = saved.getId().value();

    ResidentUpdateInformation info = validInformation();
    info.contactNumber = "09012345678";

    service.updateResident(id, info);

    CHECK(repository.findById(id)->getContactNumber() == "09012345678");

    std::remove(databasePath.c_str());
}
