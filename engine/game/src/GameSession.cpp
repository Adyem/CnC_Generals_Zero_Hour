#include "CncGame/GameSession.hpp"

#include "errno.hpp"

#include <algorithm>
#include <limits>

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
    _replay_history.clear();
    return FT_ERR_SUCCESS;
}

Error GameSession::install_default_data() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    Error error = _catalog.install_default_definitions();
    if (error != FT_ERR_SUCCESS) return error;
    error = _science_ledger.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS) return error;
    error = _special_power_ledger.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS) return error;
    return _general_roster.initialize(&_catalog);
}

Error GameSession::load_data_manifest(const char *path) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (path == nullptr) return FT_ERR_INVALID_ARGUMENT;
    Error error = _catalog.load_manifest(path);
    if (error != FT_ERR_SUCCESS) return error;
    error = _science_ledger.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS) return error;
    error = _special_power_ledger.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS) return error;
    return _general_roster.initialize(&_catalog);
}

Error GameSession::submit_world_delta(EntityId entity, int64_t delta) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    if (_next_command_sequence == std::numeric_limits<uint64_t>::max())
        return FT_ERR_OUT_OF_RANGE;
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
    error = _systems.run(SystemPhase::presentation, _world.tick());
    if (error != FT_ERR_SUCCESS) return error;
    try
    {
        _replay_history.push_back(ReplayRecord{_world.tick(), _world.canonical_state_hash()});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error GameSession::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    (void)_systems.clear();
    _commands.clear();
    _next_command_sequence = 0U;
    _replay_history.clear();
    (void)_renderer.shutdown();
    (void)_network.shutdown();
    (void)_science_ledger.shutdown();
    (void)_special_power_ledger.shutdown();
    (void)_general_roster.shutdown();
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
zero_hour::ScienceLedger &GameSession::science_ledger() noexcept { return _science_ledger; }
zero_hour::SpecialPowerLedger &GameSession::special_power_ledger() noexcept
{
    return _special_power_ledger;
}
zero_hour::GeneralRoster &GameSession::general_roster() noexcept { return _general_roster; }
void GameSession::clear_replay_history() noexcept { _replay_history.clear(); }
Error GameSession::verify_replay(const std::vector<ReplayRecord> &expected) const noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (expected.size() != _replay_history.size()) return FT_ERR_CONFIGURATION;
    for (std::size_t index = 0U; index < _replay_history.size(); ++index)
    {
        const ReplayRecord &actual = _replay_history[index];
        const ReplayRecord &reference = expected[index];
        if (actual.tick.value != reference.tick.value || actual.state_hash != reference.state_hash)
            return FT_ERR_CONFIGURATION;
    }
    return FT_ERR_SUCCESS;
}
const std::vector<GameSession::ReplayRecord> &GameSession::replay_history() const noexcept
{
    return _replay_history;
}
Renderer &GameSession::renderer() noexcept { return _renderer; }
NetworkSession &GameSession::network() noexcept { return _network; }

}
