#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include "../models/Resident.h"
#include "../models/ResidentValidator.h"

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

int main(int argc, char** argv)
{
    using namespace drogon;

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    std::thread thr([&]() {
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    f1.get();
    int status = test::run(argc, argv);

    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
