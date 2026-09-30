#pragma once

#include "../database/Database.h"
#include "../models/Resident.h"

#include <optional>
#include <vector> 

class ResidentRepository
{
public:
    explicit ResidentRepository(Database& database);

    Resident save(const Resident& resident);

    std::optional<Resident> findById(int residentId);

    std::vector<Resident> findAll();  

    std::vector<Resident> searchByName(const std::string& searchTerm);

    bool update(const Resident& resident);
    
private:
    Database& database_;

    Resident mapRowToResident(sqlite3_stmt* statement) const;
};