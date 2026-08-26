#pragma once

#include <cstdint>
#include <vector>

#include "CncRuntime/Types.hpp"

namespace cnc
{

struct WorldCommand
{
    EntityId entity;
    int64_t delta = 0;
    uint64_t sequence = 0U;
};

class DeterministicWorld final
{
public:
    DeterministicWorld() noexcept = default;
    DeterministicWorld(const DeterministicWorld &) = delete;
    DeterministicWorld &operator=(const DeterministicWorld &) = delete;

    Error initialize() noexcept;
    Error shutdown() noexcept;

    Error create_entity(EntityId *entity_id_out) noexcept;
    Error destroy_entity(EntityId entity_id) noexcept;
    Error queue_delta(EntityId entity_id, int64_t delta) noexcept;
    Error advance_one_tick() noexcept;

    SimulationTick tick() const noexcept;
    Error read_value(EntityId entity_id, int64_t *value_out) const noexcept;
    uint64_t canonical_state_hash() const noexcept;

private:
    struct EntityState
    {
        EntityId id;
        int64_t value = 0;
        Bool alive = FT_TRUE;
    };

    std::vector<EntityState> _entities;
    std::vector<WorldCommand> _pending_commands;
    EntityIdValue _next_entity_id = 1U;
    uint64_t _next_command_sequence = 0U;
    SimulationTickValue _tick = 0U;
    Bool _initialized = FT_FALSE;

    EntityState *find_entity(EntityId entity_id) noexcept;
    const EntityState *find_entity(EntityId entity_id) const noexcept;
};

}
