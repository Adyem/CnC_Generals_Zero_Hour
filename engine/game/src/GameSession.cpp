#include "CncGame/GameSession.hpp"

#include "errno.hpp"

#include <algorithm>

namespace cnc
{

GameSession::~GameSession() noexcept { (void)shutdown(); }

Error GameSession::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    Error error = _runtime.initialize();
    if (error != FT_ERR_SUCCESS) return error;
    error = _world.initialize();
    if (error != FT_ERR_SUCCESS) { (void)_runtime.shutdown(); return error; }
    error = _catalog.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _network.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _renderer.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_network.shutdown();
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error GameSession::install_default_data() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    return _catalog.install_default_definitions();
}

Error GameSession::submit_world_delta(EntityId entity, int64_t delta) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    try
    {
        _commands.push_back(WorldDeltaCommand{entity, delta, _next_command_sequence++});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error GameSession::advance_one_tick() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    std::stable_sort(_commands.begin(), _commands.end(),
        [](const WorldDeltaCommand &left, const WorldDeltaCommand &right)
        { return left.sequence < right.sequence; });
    for (const WorldDeltaCommand &command : _commands)
    {
        const Error queue_error = _world.queue_delta(command.entity, command.delta);
        if (queue_error != FT_ERR_SUCCESS) return queue_error;
    }
    _commands.clear();

    Error error = _systems.run(SystemPhase::ingest_commands, _world.tick());
    if (error != FT_ERR_SUCCESS) return error;
    error = _world.advance_one_tick();
    if (error != FT_ERR_SUCCESS) return error;
    error = _systems.run(SystemPhase::simulation, _world.tick());
    if (error != FT_ERR_SUCCESS) return error;
    return _systems.run(SystemPhase::presentation, _world.tick());
}

Error GameSession::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    (void)_systems.clear();
    _commands.clear();
    _next_command_sequence = 0U;
    (void)_renderer.shutdown();
    (void)_network.shutdown();
    (void)_catalog.shutdown();
    (void)_world.shutdown();
    const Error error = _runtime.shutdown();
    _initialized = FT_FALSE;
    return error;
}

Bool GameSession::is_initialized() const noexcept { return _initialized; }
Runtime &GameSession::runtime() noexcept { return _runtime; }
DeterministicWorld &GameSession::world() noexcept { return _world; }
SystemRegistry &GameSession::systems() noexcept { return _systems; }
const zero_hour::Catalog &GameSession::catalog() const noexcept { return _catalog; }
HeadlessRenderer &GameSession::renderer() noexcept { return _renderer; }
OfflineNetworkSession &GameSession::network() noexcept { return _network; }

}
