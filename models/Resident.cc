#include "Resident.h"

Resident::Resident(
    int id,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& address,
    const std::string& contactNumber,
    const std::string& email,
    ResidentStatus status
)
    : id(id),
      firstName(firstName),
      lastName(lastName),
      address(address),
      contactNumber(contactNumber),
      email(email),
      status(status)
{
}

int Resident::getId() const
{
    return id;
}

const std::string& Resident::getFirstName() const
{
    return firstName;
}

const std::string& Resident::getLastName() const
{
    return lastName;
}

const std::string& Resident::getAddress() const
{
    return address;
}

const std::string& Resident::getContactNumber() const
{
    return contactNumber;
}

const std::string& Resident::getEmail() const
{
    return email;
}

ResidentStatus Resident::getStatus() const
{
    return status;
}
