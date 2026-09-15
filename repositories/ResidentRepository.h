#pragma once

#include "../database/Database.h"
#include "../models/Resident.h"

#include <optional>

class ResidentRepository
{
public:
    explicit ResidentRepository(Database& database);

    Resident save(const Resident& resident);

    std::optional<Resident> findById(int residentId);

private:
    Database& database_;
};