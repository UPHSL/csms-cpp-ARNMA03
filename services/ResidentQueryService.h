#pragma once

#include "../models/Resident.h"
#include "../repositories/ResidentRepository.h"

#include <string>
#include <vector>

class ResidentQueryService
{
public:
    explicit ResidentQueryService(ResidentRepository& repository);
    
    std::vector<Resident> listResidents();

    std::vector<Resident> searchResidents(const std::string& searchTerm);

private:
    ResidentRepository& repository_;

    static std::string trim(const std::string& value);
};