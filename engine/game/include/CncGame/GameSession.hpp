#pragma once

#include <cstdint>
#include <vector>

#include "CncRuntime/Runtime.hpp"
#include "CncSimulation/SystemRegistry.hpp"
#include "CncSimulation/World.hpp"
#include "ZeroHourData/Catalog.hpp"
#include "ZeroHourData/ScienceLedger.hpp"
#include "ZeroHourData/SpecialPowerLedger.hpp"
#include "ZeroHourData/GeneralRoster.hpp"
#include "CncRender/Renderer.hpp"
#include "CncNetwork/NetworkSession.hpp"

namespace cnc
{

class GameSession final
{
public:
    struct WorldDeltaCommand
    {
        EntityId entity;
        int64_t delta = 0;
        uint64_t sequence = 0U;
    };
    struct ReplayRecord
    {
        SimulationTick tick;
        uint64_t state_hash = 0U;
    };

    GameSession() noexcept = default;
    ~GameSession() noexcept;
    GameSession(const GameSession &) = delete;
    GameSession &operator=(const GameSession &) = delete;

    Error initialize() noexcept;
    Error install_default_data() noexcept;
    // Loads game-owned definitions; the Libft core remains data-agnostic.
    Error load_data_manifest(const char *path) noexcept;
    Error submit_world_delta(EntityId entity, int64_t delta) noexcept;
    Error advance_one_tick() noexcept;
    Error shutdown() noexcept;
    void clear_replay_history() noexcept;
    Error verify_replay(const std::vector<ReplayRecord> &expected) const noexcept;
    Bool is_initialized() const noexcept;
    Runtime &runtime() noexcept;
    DeterministicWorld &world() noexcept;
    SystemRegistry &systems() noexcept;
    const zero_hour::Catalog &catalog() const noexcept;
    zero_hour::ScienceLedger &science_ledger() noexcept;
    zero_hour::SpecialPowerLedger &special_power_ledger() noexcept;
    zero_hour::GeneralRoster &general_roster() noexcept;
    const std::vector<ReplayRecord> &replay_history() const noexcept;
    HeadlessRenderer &renderer() noexcept;
    NetworkSession &network() noexcept;

private:
    Runtime _runtime;
    DeterministicWorld _world;
    SystemRegistry _systems;
    zero_hour::Catalog _catalog;
    zero_hour::ScienceLedger _science_ledger;
    zero_hour::SpecialPowerLedger _special_power_ledger;
    zero_hour::GeneralRoster _general_roster;
    HeadlessRenderer _renderer;
    OfflineNetworkSession _network;
    std::vector<WorldDeltaCommand> _commands;
    uint64_t _next_command_sequence = 0U;
    std::vector<ReplayRecord> _replay_history;
    Bool _initialized = FT_FALSE;
};

}
