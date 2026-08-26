#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include "../models/Resident.h"
#include "../models/ResidentValidator.h"

// ---- T01 tests (preserved, matching the committed Resident model) ----

DROGON_TEST(ResidentCreation)
{
    Resident resident(
        1,
        "Juan",
        "Dela Cruz",
        "Laguna",
        "09123456789",
        "juan@example.com",
        ResidentStatus::Active
    );

    CHECK(resident.getId() == 1);
    CHECK(resident.getFirstName() == "Juan");
    CHECK(resident.getLastName() == "Dela Cruz");
}

DROGON_TEST(ResidentInformationAccess)
{
    Resident resident(
        2,
        "Maria",
        "Santos",
        "Binan, Laguna",
        "09987654321",
        "maria@example.com",
        ResidentStatus::Active
    );

    CHECK(resident.getId() == 2);
    CHECK(resident.getFirstName() == "Maria");
    CHECK(resident.getLastName() == "Santos");
    CHECK(resident.getAddress() == "Binan, Laguna");
    CHECK(resident.getContactNumber() == "09987654321");
    CHECK(resident.getEmail() == "maria@example.com");
}

DROGON_TEST(ResidentStatus)
{
    Resident resident(
        3,
        "Pedro",
        "Reyes",
        "Santa Rosa, Laguna",
        "09111222333",
        "pedro@example.com",
        ResidentStatus::Active
    );

    CHECK(resident.getStatus() == ResidentStatus::Active);
}

// ---- T02 tests (Resident validation) ----

DROGON_TEST(ValidResidentInformationPassesValidation)
{
    Resident resident(
        1,
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;

    CHECK(validator.isValid(resident) == true);
}

DROGON_TEST(MissingFirstNameFailsValidation)
{
    Resident resident(
        1,
        "",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.firstNameValid == false);
}

DROGON_TEST(MissingLastNameFailsValidation)
{
    Resident resident(
        1,
        "Juan",
        "",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.lastNameValid == false);
}

DROGON_TEST(MissingAddressFailsValidation)
{
    Resident resident(
        1,
        "Juan",
        "Dela Cruz",
        "",
        "09171234567",
        "juan@example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.addressValid == false);
}

DROGON_TEST(WhitespaceOnlyRequiredInformationFailsValidation)
{
    Resident resident(
        1,
        "   ",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.firstNameValid == false);
}

DROGON_TEST(InvalidContactNumberFailsValidation)
{
    Resident resident(
        1,
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "0917ABC4567",
        "juan@example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.contactNumberValid == false);
}

DROGON_TEST(InvalidEmailFailsValidation)
{
    Resident resident(
        1,
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan.example.com",
        ResidentStatus::Active
    );

    ResidentValidator validator;
    auto result = validator.validate(resident);

    CHECK(validator.isValid(resident) == false);
    CHECK(result.emailValid == false);
}

DROGON_TEST(SupportedResidentStatusesPassValidation)
{
    Resident activeResident(
        1,
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        ResidentStatus::Active
    );

    Resident inactiveResident(
        2,
        "Maria",
        "Santos",
        "Barangay Santo Tomas",
        "09181234567",
        "maria@example.com",
        ResidentStatus::Inactive
    );

    ResidentValidator validator;

    CHECK(validator.isValid(activeResident) == true);
    CHECK(validator.isValid(inactiveResident) == true);
}

DROGON_TEST(UnsupportedResidentStatusFailsValidation)
{
    Resident resident(
        1,
        "Juan",
        "Dela Cruz",
        "Barangay Santo Tomas",
        "09171234567",
        "juan@example.com",
        static_cast<ResidentStatus>(99)
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

    // Start the main loop on another thread
    std::thread thr([&]() {
        // Queues the promise to be fulfilled after starting the loop
        app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
        app().run();
    });

    // The future is only satisfied after the event loop started
    f1.get();
    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    app().getLoop()->queueInLoop([]() { app().quit(); });
    thr.join();
    return status;
}
