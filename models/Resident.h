#pragma once

#include <optional>
#include <string>

class Resident
{
public:
    Resident(
        std::string firstName,
        std::string lastName,
        std::string address,
        std::string contactNumber,
        std::string email,
        std::string status = "Active",
        std::optional<int> id = std::nullopt
    );

    std::optional<int> getId() const;
    const std::string& getFirstName() const;
    const std::string& getLastName() const;
    const std::string& getAddress() const;
    const std::string& getContactNumber() const;
    const std::string& getEmail() const;
    const std::string& getStatus() const;

    void setFirstName(const std::string& firstName);
    void setLastName(const std::string& lastName);
    void setAddress(const std::string& address);
    void setContactNumber(const std::string& contactNumber);
    void setEmail(const std::string& email);
    void setStatus(const std::string& status);

private:
    std::optional<int> id_;
    std::string firstName_;
    std::string lastName_;
    std::string address_;
    std::string contactNumber_;
    std::string email_;
    std::string status_;
};