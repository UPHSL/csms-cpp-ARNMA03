#pragma once

#include "../models/Resident.h"
#include "../repositories/ResidentRepository.h"

#include <optional>

enum class ResidentDeactivationOutcome
{
    Deactivated,
    AlreadyInactive,
    NotFound
};

struct ResidentDeactivationResult
{
    ResidentDeactivationOutcome outcome;
    std::optional<Resident> resident; // set for Deactivated and AlreadyInactive; empty for NotFound
};

class ResidentDeactivationService
{
public:
    explicit ResidentDeactivationService(ResidentRepository& repository);

    ResidentDeactivationResult deactivate(int residentId);

private:
    ResidentRepository& repository_;
};
