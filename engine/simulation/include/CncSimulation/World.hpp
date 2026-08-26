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

class SimulationWorld
{
public:
    virtual ~SimulationWorld() noexcept = default;
    virtual Error initialize() noexcept = 0;
    virtual const char *backend_name() const noexcept = 0;
    virtual Error shutdown() noexcept = 0;
    virtual Error create_entity(EntityId *entity_id_out) noexcept = 0;
    virtual Error destroy_entity(EntityId entity_id) noexcept = 0;
    virtual Error queue_delta(EntityId entity_id, int64_t delta) noexcept = 0;
    virtual Error advance_one_tick() noexcept = 0;
    virtual SimulationTick tick() const noexcept = 0;
    virtual Error read_value(EntityId entity_id, int64_t *value_out) const noexcept = 0;
    virtual uint64_t canonical_state_hash() const noexcept = 0;
};

class DeterministicWorld final : public SimulationWorld
{
public:
    DeterministicWorld() noexcept = default;
    DeterministicWorld(const DeterministicWorld &) = delete;
    DeterministicWorld &operator=(const DeterministicWorld &) = delete;

    Error initialize() noexcept override;
    const char *backend_name() const noexcept override;
    Error shutdown() noexcept override;

    Error create_entity(EntityId *entity_id_out) noexcept override;
    Error destroy_entity(EntityId entity_id) noexcept override;
    Error queue_delta(EntityId entity_id, int64_t delta) noexcept override;
    Error advance_one_tick() noexcept override;

    SimulationTick tick() const noexcept override;
    Error read_value(EntityId entity_id, int64_t *value_out) const noexcept override;
    uint64_t canonical_state_hash() const noexcept override;

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
