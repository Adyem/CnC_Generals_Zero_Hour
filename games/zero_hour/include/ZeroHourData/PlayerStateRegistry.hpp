#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/PlayerRegistry.hpp"
#include "ZeroHourData/PlayerState.hpp"

namespace zero_hour
{

// Match-level owner of game-specific state. Generic player identity remains
// in Libft/engine PlayerRegistry; this table owns one Zero Hour composition per
// identity and shares the game catalog/ledgers explicitly.
class PlayerStateRegistry final
{
public:
    struct SnapshotEntry
    {
        cnc::PlayerId player;
        cnc::DefinitionId faction;
        uint32_t science_points = 0U;
    };
    struct Snapshot
    {
        uint32_t schema_version = 1U;
        std::vector<SnapshotEntry> entries;
    };
    cnc::Error initialize(const Catalog *catalog, ScienceLedger *science,
                          SpecialPowerLedger *powers, GeneralRoster *generals) noexcept;
    cnc::Error create(cnc::PlayerId player) noexcept;
    cnc::Error remove(cnc::PlayerId player) noexcept;
    PlayerState *find(cnc::PlayerId player) noexcept;
    const PlayerState *find(cnc::PlayerId player) const noexcept;
    cnc::Size size() const noexcept;
    uint64_t canonical_state_hash() const noexcept;
    cnc::Error export_snapshot(Snapshot *snapshot_out) const noexcept;
    cnc::Error import_snapshot(const Snapshot &snapshot) noexcept;
    void swap(PlayerStateRegistry &other) noexcept;
    void rebind_generals(GeneralRoster *generals) noexcept;
    void rebind_powers(SpecialPowerLedger *powers) noexcept;
    void rebind_science(ScienceLedger *science) noexcept;
    cnc::Error shutdown() noexcept;

private:
    struct Entry
    {
        cnc::PlayerId player;
        PlayerState state;
    };
    const Catalog *_catalog = nullptr;
    ScienceLedger *_science = nullptr;
    SpecialPowerLedger *_powers = nullptr;
    GeneralRoster *_generals = nullptr;
    std::vector<Entry> _entries;
    bool _initialized = false;
};

}
