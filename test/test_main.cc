#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include "../models/resident.h"

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
