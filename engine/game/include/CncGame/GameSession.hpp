#pragma once

#include "CncRuntime/Runtime.hpp"
#include "CncSimulation/SystemRegistry.hpp"
#include "CncSimulation/World.hpp"
#include "ZeroHourData/Catalog.hpp"

namespace cnc
{

class GameSession final
{
public:
    GameSession() noexcept = default;
    ~GameSession() noexcept;
    GameSession(const GameSession &) = delete;
    GameSession &operator=(const GameSession &) = delete;

    Error initialize() noexcept;
    Error install_default_data() noexcept;
    Error advance_one_tick() noexcept;
    Error shutdown() noexcept;
    Bool is_initialized() const noexcept;
    Runtime &runtime() noexcept;
    DeterministicWorld &world() noexcept;
    SystemRegistry &systems() noexcept;
    const zero_hour::Catalog &catalog() const noexcept;

private:
    Runtime _runtime;
    DeterministicWorld _world;
    SystemRegistry _systems;
    zero_hour::Catalog _catalog;
    Bool _initialized = FT_FALSE;
};

}
