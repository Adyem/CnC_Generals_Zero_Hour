#pragma once

#include <cstdint>
#include <vector>

#include "ZeroHourData/Catalog.hpp"
#include "CncRuntime/Types.hpp"

namespace zero_hour
{

class SpecialPowerLedger final
{
public:
    struct SnapshotEntry { cnc::DefinitionId power; cnc::SimulationTick ready_at; };
    struct Snapshot { uint32_t schema_version = 1U; std::vector<SnapshotEntry> cooldowns; };
    cnc::Error initialize(const Catalog *catalog) noexcept;
    cnc::Error activate(cnc::DefinitionId power, cnc::SimulationTick now,
                        cnc::SimulationTick *ready_at) noexcept;
    bool is_ready(cnc::DefinitionId power, cnc::SimulationTick now) const noexcept;
    cnc::Size active_count() const noexcept;
    cnc::Error export_snapshot(Snapshot *snapshot_out) const noexcept;
    cnc::Error import_snapshot(const Snapshot &snapshot) noexcept;
    void swap(SpecialPowerLedger &other) noexcept;
    cnc::Error shutdown() noexcept;

private:
    struct Cooldown
    {
        cnc::DefinitionId power;
        cnc::SimulationTick ready_at;
    };
    const Catalog *_catalog = nullptr;
    std::vector<Cooldown> _cooldowns;
    bool _initialized = false;
};

}
