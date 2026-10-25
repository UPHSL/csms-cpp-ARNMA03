#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../models/ResidentValidator.h"
#include "../database/Database.h"
#include "../repositories/ResidentRepository.h"
#include "../services/ResidentRegistrationService.h"

#include <cstdio>
#include <string>

// ---- T04 tests (Resident registration) ----

DROGON_TEST(RegisterValidResidentSucceeds)
{
    const std::string databasePath = "test_register_valid.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "Liza",
        "Bautista",
        "Barangay Uno",
        "09171234567",
        "liza@example.com"
    );

    auto result = service.registerResident(resident);

    CHECK(result.success == true);

    std::remove(databasePath.c_str());
}

DROGON_TEST(RegisteredResidentReceivesIdentifier)
{
    const std::string databasePath = "test_register_id.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "Noel",
        "Ramos",
        "Barangay Dos",
        "09181234567",
        "noel@example.com"
    );

    CHECK(!resident.getId().has_value());

    auto result = service.registerResident(resident);

    CHECK(result.success == true);
    CHECK(result.resident.has_value());
    CHECK(result.resident->getId().has_value());
    CHECK(result.resident->getId().value() > 0);

    std::remove(databasePath.c_str());
}

DROGON_TEST(RegisteredResidentIsPersisted)
{
    const std::string databasePath = "test_register_persisted.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "Grace",
        "Torres",
        "Barangay Tres",
        "09191234567",
        "grace@example.com"
    );

    auto result = service.registerResident(resident);
    int residentId = result.resident->getId().value();

    auto foundResident = repository.findById(residentId);

    CHECK(foundResident.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(RegisteredResidentInformationIsPreserved)
{
    const std::string databasePath = "test_register_preserved.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "Miguel",
        "Aquino",
        "123 Rizal Street",
        "09201234567",
        "miguel@example.com"
    );

    auto result = service.registerResident(resident);
    int residentId = result.resident->getId().value();

    auto foundResident = repository.findById(residentId);

    CHECK(foundResident.has_value());
    CHECK(foundResident->getFirstName() == "Miguel");
    CHECK(foundResident->getLastName() == "Aquino");
    CHECK(foundResident->getAddress() == "123 Rizal Street");
    CHECK(foundResident->getContactNumber() == "09201234567");
    CHECK(foundResident->getEmail() == "miguel@example.com");
    CHECK(foundResident->getStatus() == "Active");

    std::remove(databasePath.c_str());
}

DROGON_TEST(RegisteredResidentDefaultActiveStatusIsPreserved)
{
    const std::string databasePath = "test_register_active.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "Elena",
        "Fernandez",
        "Barangay Cuatro",
        "09211234567",
        "elena@example.com"
    );

    auto result = service.registerResident(resident);
    int residentId = result.resident->getId().value();

    auto foundResident = repository.findById(residentId);

    CHECK(foundResident.has_value());
    CHECK(foundResident->getStatus() == "Active");

    std::remove(databasePath.c_str());
}

DROGON_TEST(InvalidResidentRegistrationFails)
{
    const std::string databasePath = "test_register_invalid.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "",
        "Dela Cruz",
        "Barangay Uno",
        "09171234567",
        "juan@example.com"
    );

    auto result = service.registerResident(resident);

    CHECK(result.success == false);

    std::remove(databasePath.c_str());
}

DROGON_TEST(InvalidResidentIsNotPersisted)
{
    const std::string databasePath = "test_register_not_persisted.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Uno",
        "0917ABC4567", // invalid contact number
        "juan@example.com"
    );

    auto result = service.registerResident(resident);

    CHECK(result.success == false);
    CHECK(!result.resident.has_value());

    // Fresh database, so a successful save would have produced id 1.
    auto foundResident = repository.findById(1);
    CHECK(!foundResident.has_value());

    std::remove(databasePath.c_str());
}

DROGON_TEST(ValidationFailureCanBeIdentified)
{
    const std::string databasePath = "test_register_failure_field.db";
    std::remove(databasePath.c_str());

    Database database(databasePath);
    database.initialize();

    ResidentValidator validator;
    ResidentRepository repository(database);
    ResidentRegistrationService service(validator, repository);

    Resident resident(
        "", // invalid first name
        "Dela Cruz",
        "Barangay Uno",
        "09171234567",
        "juan@example.com"
    );

    auto result = service.registerResident(resident);

    CHECK(result.success == false);
    CHECK(result.validationResult.firstNameValid == false);
    CHECK(result.validationResult.lastNameValid == true);

    std::remove(databasePath.c_str());
}