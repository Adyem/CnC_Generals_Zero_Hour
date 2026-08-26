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
    error = _players.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _spatial.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_players.shutdown();
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _combat.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_spatial.shutdown();
        (void)_players.shutdown();
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _visibility.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_combat.shutdown();
        (void)_spatial.shutdown();
        (void)_players.shutdown();
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _network.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_catalog.shutdown();
        (void)_players.shutdown();
        (void)_spatial.shutdown();
        (void)_combat.shutdown();
        (void)_visibility.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    error = _renderer.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_network.shutdown();
        (void)_players.shutdown();
        (void)_spatial.shutdown();
        (void)_combat.shutdown();
        (void)_visibility.shutdown();
        (void)_catalog.shutdown();
        (void)_world.shutdown();
        (void)_runtime.shutdown();
        return error;
    }
    _initialized = FT_TRUE;
    _phase = Phase::initialized;
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
    error = _general_roster.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS) return error;
    error = _player_state.initialize(&_catalog, &_science_ledger,
                                     &_special_power_ledger, &_general_roster);
    if (error != FT_ERR_SUCCESS) return error;
    error = _player_states.initialize(&_catalog, &_science_ledger,
                                      &_special_power_ledger, &_general_roster);
    if (error != FT_ERR_SUCCESS)
    {
        (void)_player_state.shutdown();
        return error;
    }
    error = validate_game_data();
    if (error == FT_ERR_SUCCESS) _phase = Phase::data_ready;
    return error;
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
    error = _general_roster.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS) return error;
    error = _player_state.initialize(&_catalog, &_science_ledger,
                                     &_special_power_ledger, &_general_roster);
    if (error != FT_ERR_SUCCESS) return error;
    error = _player_states.initialize(&_catalog, &_science_ledger,
                                      &_special_power_ledger, &_general_roster);
    if (error != FT_ERR_SUCCESS)
    {
        (void)_player_state.shutdown();
        return error;
    }
    error = validate_game_data();
    if (error == FT_ERR_SUCCESS) _phase = Phase::data_ready;
    return error;
}

Error GameSession::submit_world_delta(EntityId entity, int64_t delta) noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
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

Error GameSession::submit_command_frame(const uint8_t *bytes, Size byte_count) noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
    WorldCommandFrame frame;
    const Error decode_error = WorldCommandCodec::decode(bytes, byte_count, &frame);
    if (decode_error != FT_ERR_SUCCESS) return decode_error;
    if (frame.tick.value != _world.tick().value) return FT_ERR_INVALID_OPERATION;
    using CommandCount = std::vector<WorldDeltaCommand>::size_type;
    const CommandCount current_count = _commands.size();
    const CommandCount incoming_count = frame.commands.size();
    if (incoming_count > std::numeric_limits<CommandCount>::max() - current_count)
        return FT_ERR_OUT_OF_RANGE;
    std::vector<WorldDeltaCommand> projected;
    uint64_t next_sequence = _next_command_sequence;
    try
    {
        projected = _commands;
        projected.reserve(_commands.size() + frame.commands.size());
        for (const WorldCommand &command : frame.commands)
        {
            if (!command.entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
            int64_t current_value = 0;
            const Error entity_error = _world.read_value(command.entity, &current_value);
            if (entity_error != FT_ERR_SUCCESS) return entity_error;
            if (next_sequence == std::numeric_limits<uint64_t>::max())
                return FT_ERR_OUT_OF_RANGE;
            projected.push_back(WorldDeltaCommand{command.entity, command.delta, next_sequence++});
        }
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _commands.swap(projected);
    _next_command_sequence = next_sequence;
    return FT_ERR_SUCCESS;
}

Error GameSession::advance_one_tick() noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
    std::stable_sort(_commands.begin(), _commands.end(),
        [](const WorldDeltaCommand &left, const WorldDeltaCommand &right)
        { return left.sequence < right.sequence; });
    for (const WorldDeltaCommand &command : _commands)
    {
        const Error queue_error = _world.queue_delta(command.entity, command.delta);
        if (queue_error != FT_ERR_SUCCESS)
        {
            (void)_world.discard_pending_commands();
            return queue_error;
        }
    }

    Error error = _systems.run(SystemPhase::ingest_commands, _world.tick());
    if (error != FT_ERR_SUCCESS)
    {
        (void)_world.discard_pending_commands();
        return error;
    }
    error = _world.advance_one_tick();
    if (error != FT_ERR_SUCCESS)
    {
        (void)_world.discard_pending_commands();
        return error;
    }
    _commands.clear();
    error = _systems.run(SystemPhase::simulation, _world.tick());
    if (error != FT_ERR_SUCCESS) return error;
    error = _systems.run(SystemPhase::presentation, _world.tick());
    if (error != FT_ERR_SUCCESS) return error;
    WorldSnapshot snapshot;
    error = _world.export_snapshot(&snapshot);
    if (error != FT_ERR_SUCCESS) return error;
    error = _renderer.begin_frame();
    if (error != FT_ERR_SUCCESS) return error;
    error = _renderer.present_snapshot(snapshot);
    if (error != FT_ERR_SUCCESS) return error;
    error = _renderer.end_frame();
    if (error != FT_ERR_SUCCESS) return error;
    try
    {
        _replay_history.push_back(ReplayRecord{_world.tick(), canonical_state_hash()});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _phase = Phase::running;
    return FT_ERR_SUCCESS;
}

Error GameSession::save_snapshot(std::vector<uint8_t> *bytes_out) const noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
    SessionSnapshot snapshot;
    Error error = _world.export_snapshot(&snapshot.world);
    if (error != FT_ERR_SUCCESS) return error;
    error = _players.export_snapshot(&snapshot.players);
    if (error != FT_ERR_SUCCESS) return error;
    error = _player_states.export_snapshot(&snapshot.player_states);
    if (error != FT_ERR_SUCCESS) return error;
    error = _general_roster.export_snapshot(&snapshot.generals);
    if (error != FT_ERR_SUCCESS) return error;
    error = _special_power_ledger.export_snapshot(&snapshot.powers);
    if (error != FT_ERR_SUCCESS) return error;
    error = _spatial.export_snapshot(&snapshot.spatial);
    if (error != FT_ERR_SUCCESS) return error;
    error = _combat.export_snapshot(&snapshot.combat);
    if (error != FT_ERR_SUCCESS) return error;
    error = _visibility.export_snapshot(&snapshot.visibility);
    if (error != FT_ERR_SUCCESS) return error;
    return SessionSnapshotCodec::encode(snapshot, bytes_out);
}

Error GameSession::load_snapshot(const uint8_t *bytes, Size byte_count) noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
    SessionSnapshot snapshot;
    Error error = SessionSnapshotCodec::decode(bytes, byte_count, &snapshot);
    if (error != FT_ERR_SUCCESS) return error;
    PlayerRegistry projected_players;
    error = projected_players.initialize();
    if (error != FT_ERR_SUCCESS) return error;
    error = projected_players.import_snapshot(snapshot.players);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_players.shutdown();
        return error;
    }
    zero_hour::GeneralRoster projected_generals;
    error = projected_generals.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_players.shutdown();
        return error;
    }
    error = projected_generals.import_snapshot(snapshot.generals);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_generals.shutdown();
        (void)projected_players.shutdown();
        return error;
    }
    zero_hour::SpecialPowerLedger projected_powers;
    error = projected_powers.initialize(&_catalog);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_generals.shutdown();
        (void)projected_players.shutdown();
        return error;
    }
    error = projected_powers.import_snapshot(snapshot.powers);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_powers.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_players.shutdown();
        return error;
    }
    zero_hour::PlayerStateRegistry projected_player_states;
    error = projected_player_states.initialize(&_catalog, &_science_ledger,
                                               &projected_powers, &projected_generals);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_players.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_powers.shutdown();
        return error;
    }
    error = projected_player_states.import_snapshot(snapshot.player_states);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_player_states.shutdown();
        (void)projected_players.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_powers.shutdown();
        return error;
    }
    SpatialIndex projected_spatial;
    error = projected_spatial.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_powers.shutdown();
        return error;
    }
    error = projected_spatial.import_snapshot(snapshot.spatial);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_spatial.shutdown();
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_powers.shutdown();
        return error;
    }
    CombatRegistry projected_combat;
    error = projected_combat.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_spatial.shutdown();
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_powers.shutdown();
        return error;
    }
    error = projected_combat.import_snapshot(snapshot.combat);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_combat.shutdown();
        (void)projected_spatial.shutdown();
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        (void)projected_generals.shutdown();
        (void)projected_powers.shutdown();
        return error;
    }
    VisibilityRegistry projected_visibility;
    error = projected_visibility.initialize();
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_combat.shutdown();
        (void)projected_spatial.shutdown();
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        (void)projected_generals.shutdown();
        return error;
    }
    error = projected_visibility.import_snapshot(snapshot.visibility);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_visibility.shutdown();
        (void)projected_combat.shutdown();
        (void)projected_spatial.shutdown();
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        return error;
    }
    error = _world.import_snapshot(snapshot.world);
    if (error != FT_ERR_SUCCESS)
    {
        (void)projected_spatial.shutdown();
        (void)projected_combat.shutdown();
        (void)projected_visibility.shutdown();
        (void)projected_players.shutdown();
        (void)projected_player_states.shutdown();
        return error;
    }
    _players.swap(projected_players);
    _general_roster.swap(projected_generals);
    _special_power_ledger.swap(projected_powers);
    _player_states.swap(projected_player_states);
    _player_states.rebind_generals(&_general_roster);
    _player_states.rebind_powers(&_special_power_ledger);
    _spatial.swap(projected_spatial);
    _combat.swap(projected_combat);
    _visibility.swap(projected_visibility);
    (void)projected_players.shutdown();
    (void)projected_player_states.shutdown();
    (void)projected_generals.shutdown();
    (void)projected_powers.shutdown();
    (void)projected_spatial.shutdown();
    (void)projected_combat.shutdown();
    (void)projected_visibility.shutdown();
    _commands.clear();
    _next_command_sequence = 0U;
    _replay_history.clear();
    _phase = snapshot.world.tick.value == 0U ? Phase::data_ready : Phase::running;
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
    (void)_player_state.shutdown();
    (void)_player_states.shutdown();
    (void)_special_power_ledger.shutdown();
    (void)_general_roster.shutdown();
    (void)_players.shutdown();
    (void)_spatial.shutdown();
    (void)_combat.shutdown();
    (void)_visibility.shutdown();
    (void)_catalog.shutdown();
    (void)_world.shutdown();
    const Error error = _runtime.shutdown();
    _initialized = FT_FALSE;
    _phase = Phase::cold;
    return error;
}

Bool GameSession::is_initialized() const noexcept { return _initialized; }
uint64_t GameSession::canonical_state_hash() const noexcept
{
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    mix(_world.canonical_state_hash());
    mix(_players.canonical_state_hash());
    mix(_spatial.canonical_state_hash());
    mix(_combat.canonical_state_hash());
    mix(_visibility.canonical_state_hash());
    mix(_player_states.canonical_state_hash());
    return hash;
}
Bool GameSession::has_game_data() const noexcept
{
    return (_initialized == FT_TRUE && _catalog.definition_count() != 0U) ? FT_TRUE : FT_FALSE;
}
Error GameSession::validate_game_data() const noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (!has_game_data()) return FT_ERR_INVALID_STATE;
    ValidationReport report;
    return _catalog.validate(report);
}
GameSession::Phase GameSession::phase() const noexcept { return _phase; }
Runtime &GameSession::runtime() noexcept { return _runtime; }
SimulationWorld &GameSession::world() noexcept { return _world; }
SystemRegistry &GameSession::systems() noexcept { return _systems; }
PlayerRegistry &GameSession::players() noexcept { return _players; }
Error GameSession::create_player(PlayerId player) noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
    Error error = _players.create_player(player);
    if (error != FT_ERR_SUCCESS) return error;
    error = _player_states.create(player);
    if (error != FT_ERR_SUCCESS)
    {
        (void)_players.remove_player(player);
        return error;
    }
    return FT_ERR_SUCCESS;
}
Error GameSession::remove_player(PlayerId player) noexcept
{
    if (_initialized != FT_TRUE ||
        (_phase != Phase::data_ready && _phase != Phase::running))
        return FT_ERR_INVALID_STATE;
    if (_players.contains(player) != FT_TRUE) return FT_ERR_NOT_FOUND;
    Error error = _player_states.remove(player);
    if (error != FT_ERR_SUCCESS) return error;
    error = _players.remove_player(player);
    return error;
}
SpatialIndex &GameSession::spatial() noexcept { return _spatial; }
CombatRegistry &GameSession::combat() noexcept { return _combat; }
VisibilityRegistry &GameSession::visibility() noexcept { return _visibility; }
const zero_hour::Catalog &GameSession::catalog() const noexcept { return _catalog; }
zero_hour::ScienceLedger &GameSession::science_ledger() noexcept { return _science_ledger; }
zero_hour::SpecialPowerLedger &GameSession::special_power_ledger() noexcept
{
    return _special_power_ledger;
}
zero_hour::GeneralRoster &GameSession::general_roster() noexcept { return _general_roster; }
zero_hour::PlayerState &GameSession::player_state() noexcept { return _player_state; }
zero_hour::PlayerStateRegistry &GameSession::player_states() noexcept { return _player_states; }
void GameSession::clear_replay_history() noexcept { _replay_history.clear(); }
Error GameSession::verify_replay(const std::vector<ReplayRecord> &expected) const noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    if (expected.size() != _replay_history.size()) return FT_ERR_CONFIGURATION;
    for (ft_size_t index = 0U; index < _replay_history.size(); ++index)
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
