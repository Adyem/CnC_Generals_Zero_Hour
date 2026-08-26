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
    cnc::Error initialize(const Catalog *catalog, ScienceLedger *science,
                          SpecialPowerLedger *powers, GeneralRoster *generals) noexcept;
    cnc::Error create(cnc::PlayerId player) noexcept;
    cnc::Error remove(cnc::PlayerId player) noexcept;
    PlayerState *find(cnc::PlayerId player) noexcept;
    const PlayerState *find(cnc::PlayerId player) const noexcept;
    cnc::Size size() const noexcept;
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
