#include "CncGame/GameSession.hpp"

#include "errno.hpp"

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
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error GameSession::install_default_data() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
    return _catalog.install_default_definitions();
}

Error GameSession::advance_one_tick() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_INVALID_STATE;
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

}
