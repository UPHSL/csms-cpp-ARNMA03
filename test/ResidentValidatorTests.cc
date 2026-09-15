#include <drogon/drogon_test.h>
#include "../models/Resident.h"
#include "../models/ResidentValidator.h"

// ---- T02 tests (Resident validation) ----

DROGON_TEST(ValidResidentInformationPassesValidation)
{
    Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    ResidentValidator validator;

    CHECK(validator.isValid(resident) == true);
}

DROGON_TEST(MissingFirstNameFailsValidation)
{
    Resident resident(
        "",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.firstNameValid == false);
}

DROGON_TEST(MissingLastNameFailsValidation)
{
    Resident resident(
        "Juan",
        "",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.lastNameValid == false);
}

DROGON_TEST(MissingAddressFailsValidation)
{
    Resident resident(
        "Juan",
        "Dela Cruz",
        "",
        "09171234567",
        "juan@example.com"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.addressValid == false);
}

DROGON_TEST(WhitespaceOnlyRequiredInformationFailsValidation)
{
    Resident resident(
        "   ",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.firstNameValid == false);
}

DROGON_TEST(InvalidContactNumberFailsValidation)
{
    Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "0917ABC4567",
        "juan@example.com"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.contactNumberValid == false);
}

DROGON_TEST(InvalidEmailFailsValidation)
{
    Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan.example.com"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.emailValid == false);
}

DROGON_TEST(SupportedResidentStatusesPassValidation)
{
    Resident activeResident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        "Active"
    );

    Resident inactiveResident(
        "Maria",
        "Santos",
        "Barangay Santo Tomas",
        "09181234567",
        "maria@example.com",
        "Inactive"
    );

    ResidentValidator validator;

    CHECK(validator.isValid(activeResident) == true);
    CHECK(validator.isValid(inactiveResident) == true);
}

DROGON_TEST(UnsupportedResidentStatusFailsValidation)
{
    Resident resident(
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        "Unknown"
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.statusValid == false);
}