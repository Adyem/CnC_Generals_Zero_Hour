#pragma once

#include "CncRuntime/Types.hpp"
#include "ZeroHourData/Catalog.hpp"
#include "ZeroHourData/GeneralRoster.hpp"
#include "ZeroHourData/ScienceLedger.hpp"
#include "ZeroHourData/SpecialPowerLedger.hpp"

namespace zero_hour
{

// Game-owned player composition. Libft supplies the generic registries and
// ticks; this class owns the meaning of faction selection and progression.
class PlayerState final
{
public:
    cnc::Error initialize(const Catalog *catalog, ScienceLedger *science,
                          SpecialPowerLedger *powers, GeneralRoster *generals) noexcept;
    cnc::Error set_faction(cnc::DefinitionId faction) noexcept;
    cnc::Error set_science_points(uint32_t points) noexcept;
    cnc::Error purchase_science(cnc::DefinitionId science) noexcept;
    cnc::Error assign_general(cnc::EntityId entity, cnc::DefinitionId general) noexcept;
    cnc::Error activate_power(cnc::DefinitionId power, cnc::SimulationTick now,
                              cnc::SimulationTick *ready_at) noexcept;
    cnc::DefinitionId faction() const noexcept;
    uint32_t science_points() const noexcept;
    cnc::Error shutdown() noexcept;

private:
    const Catalog *_catalog = nullptr;
    ScienceLedger *_science = nullptr;
    SpecialPowerLedger *_powers = nullptr;
    GeneralRoster *_generals = nullptr;
    cnc::DefinitionId _faction;
    uint32_t _science_points = 0U;
    bool _initialized = false;
};

}
