#pragma once

#include <string>

enum class ResidentStatus
{
    Active,
    Inactive
};

class Resident
{
private:
    int id;
    std::string firstName;
    std::string lastName;
    std::string address;
    std::string contactNumber;
    std::string email;
    ResidentStatus status;

public:
    Resident(
        int id,
        const std::string& firstName,
        const std::string& lastName,
        const std::string& address,
        const std::string& contactNumber,
        const std::string& email,
        ResidentStatus status
    );

    int getId() const;
    const std::string& getFirstName() const;
    const std::string& getLastName() const;
    const std::string& getAddress() const;
    const std::string& getContactNumber() const;
    const std::string& getEmail() const;
    ResidentStatus getStatus() const;
};
