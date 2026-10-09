#include "Resident.h"

#include <utility>

Resident::Resident(
    std::string firstName,
    std::string lastName,
    std::string address,
    std::string contactNumber,
    std::string email,
    std::string status,
    std::optional<int> id
)
    : id_(id),
      firstName_(std::move(firstName)),
      lastName_(std::move(lastName)),
      address_(std::move(address)),
      contactNumber_(std::move(contactNumber)),
      email_(std::move(email)),
      status_(std::move(status))
{
}

std::optional<int> Resident::getId() const
{
    return id_;
}

const std::string& Resident::getFirstName() const
{
    return firstName_;
}

const std::string& Resident::getLastName() const
{
    return lastName_;
}

const std::string& Resident::getAddress() const
{
    return address_;
}

const std::string& Resident::getContactNumber() const
{
    return contactNumber_;
}

const std::string& Resident::getEmail() const
{
    return email_;
}

const std::string& Resident::getStatus() const
{
    return status_;
}

void Resident::setFirstName(const std::string& firstName)
{
    firstName_ = firstName;
}

void Resident::setLastName(const std::string& lastName)
{
    lastName_ = lastName;
}

void Resident::setAddress(const std::string& address)
{
    address_ = address;
}

void Resident::setContactNumber(const std::string& contactNumber)
{
    contactNumber_ = contactNumber;
}

void Resident::setEmail(const std::string& email)
{
    email_ = email;
}

void Resident::setStatus(const std::string& status)
{
    status_ = status;
}