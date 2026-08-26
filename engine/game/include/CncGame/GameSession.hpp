#pragma once

#include <cstdint>
#include <vector>

#include "CncRuntime/Runtime.hpp"
#include "CncSimulation/SystemRegistry.hpp"
#include "CncSimulation/World.hpp"
#include "ZeroHourData/Catalog.hpp"
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
    Error submit_world_delta(EntityId entity, int64_t delta) noexcept;
    Error advance_one_tick() noexcept;
    Error shutdown() noexcept;
    void clear_replay_history() noexcept;
    Bool is_initialized() const noexcept;
    Runtime &runtime() noexcept;
    DeterministicWorld &world() noexcept;
    SystemRegistry &systems() noexcept;
    const zero_hour::Catalog &catalog() const noexcept;
    const std::vector<ReplayRecord> &replay_history() const noexcept;
    HeadlessRenderer &renderer() noexcept;
    OfflineNetworkSession &network() noexcept;

private:
    Runtime _runtime;
    DeterministicWorld _world;
    SystemRegistry _systems;
    zero_hour::Catalog _catalog;
    HeadlessRenderer _renderer;
    OfflineNetworkSession _network;
    std::vector<WorldDeltaCommand> _commands;
    uint64_t _next_command_sequence = 0U;
    std::vector<ReplayRecord> _replay_history;
    Bool _initialized = FT_FALSE;
};

}
