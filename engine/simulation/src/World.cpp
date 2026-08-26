#include "CncSimulation/World.hpp"

#include <algorithm>
#include <limits>

namespace cnc
{

const char *DeterministicWorld::backend_name() const noexcept
{
    return "deterministic-scaffold";
}

Error DeterministicWorld::initialize() noexcept
{
    if (_initialized == FT_TRUE)
        return FT_ERR_ALREADY_INITIALISED;
    _entities.clear();
    _pending_commands.clear();
    _next_entity_id = 1U;
    _next_command_sequence = 0U;
    _tick = 0U;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::shutdown() noexcept
{
    _pending_commands.clear();
    _entities.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::create_entity(EntityId *entity_id_out) noexcept
{
    if (_initialized != FT_TRUE)
        return FT_ERR_NOT_INITIALISED;
    if (entity_id_out == ft_nullptr)
        return FT_ERR_INVALID_POINTER;
    if (_next_entity_id == std::numeric_limits<EntityIdValue>::max())
        return FT_ERR_OUT_OF_RANGE;

    const EntityId entity_id{_next_entity_id++};
    try
    {
        _entities.push_back(EntityState{entity_id, 0, FT_TRUE});
    }
    catch (...)
    {
        --_next_entity_id;
        return FT_ERR_NO_MEMORY;
    }
    *entity_id_out = entity_id;
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::destroy_entity(EntityId entity_id) noexcept
{
    EntityState *const entity = find_entity(entity_id);
    if (entity == nullptr)
        return FT_ERR_NOT_FOUND;
    entity->alive = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::queue_delta(EntityId entity_id, int64_t delta) noexcept
{
    if (_initialized != FT_TRUE)
        return FT_ERR_NOT_INITIALISED;
    if (find_entity(entity_id) == nullptr)
        return FT_ERR_NOT_FOUND;
    if (_next_command_sequence == std::numeric_limits<uint64_t>::max())
        return FT_ERR_OUT_OF_RANGE;
    try
    {
        _pending_commands.push_back(WorldCommand{entity_id, delta,
                                                  _next_command_sequence++});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::discard_pending_commands() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    _pending_commands.clear();
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::advance_one_tick() noexcept
{
    if (_initialized != FT_TRUE)
        return FT_ERR_NOT_INITIALISED;

    std::stable_sort(_pending_commands.begin(), _pending_commands.end(),
        [](const WorldCommand &left, const WorldCommand &right)
        {
            return left.sequence < right.sequence;
        });
    // Preflight against a shadow copy before mutating live state. This handles
    // cumulative overflow across multiple commands and keeps the tick atomic.
    std::vector<EntityState> projected;
    try { projected = _entities; }
    catch (...) { return FT_ERR_NO_MEMORY; }
    for (const WorldCommand &command : _pending_commands)
    {
        EntityState *entity = nullptr;
        for (EntityState &candidate : projected)
        {
            if (candidate.id.value == command.entity.value)
            {
                entity = &candidate;
                break;
            }
        }
        if (entity == nullptr || entity->alive != FT_TRUE)
            continue;
        if ((command.delta > 0 &&
             entity->value > std::numeric_limits<int64_t>::max() - command.delta) ||
            (command.delta < 0 &&
             entity->value < std::numeric_limits<int64_t>::min() - command.delta))
            return FT_ERR_OUT_OF_RANGE;
        entity->value += command.delta;
    }
    if (_tick == std::numeric_limits<SimulationTickValue>::max())
        return FT_ERR_OUT_OF_RANGE;
    _entities.swap(projected);
    _pending_commands.clear();
    ++_tick;
    return FT_ERR_SUCCESS;
}

SimulationTick DeterministicWorld::tick() const noexcept
{
    return SimulationTick{_tick};
}

Error DeterministicWorld::read_value(EntityId entity_id, int64_t *value_out) const noexcept
{
    if (value_out == ft_nullptr)
        return FT_ERR_INVALID_POINTER;
    const EntityState *const entity = find_entity(entity_id);
    if (entity == nullptr || entity->alive != FT_TRUE)
        return FT_ERR_NOT_FOUND;
    *value_out = entity->value;
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::export_snapshot(WorldSnapshot *snapshot_out) const noexcept
{
    if (snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        snapshot_out->tick = tick();
        snapshot_out->entries.clear();
        snapshot_out->entries.reserve(_entities.size());
        for (const EntityState &entity : _entities)
            snapshot_out->entries.push_back(WorldSnapshotEntry{entity.id, entity.value, entity.alive});
    }
    catch (...)
    {
        snapshot_out->entries.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error DeterministicWorld::import_snapshot(const WorldSnapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U ||
        snapshot.entries.size() > static_cast<std::vector<WorldSnapshotEntry>::size_type>(1U << 20U))
        return FT_ERR_CONFIGURATION;
    std::vector<EntityState> restored;
    EntityIdValue next_id = 1U;
    try
    {
        restored.reserve(snapshot.entries.size());
        EntityIdValue previous_id = 0U;
        for (const WorldSnapshotEntry &entry : snapshot.entries)
        {
            if (!entry.entity.is_valid() || entry.entity.value <= previous_id ||
                (entry.alive != FT_TRUE && entry.alive != FT_FALSE))
                return FT_ERR_CONFIGURATION;
            restored.push_back(EntityState{entry.entity, entry.value, entry.alive});
            previous_id = entry.entity.value;
            if (previous_id == std::numeric_limits<EntityIdValue>::max())
                return FT_ERR_OUT_OF_RANGE;
            next_id = previous_id + 1U;
        }
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _entities.swap(restored);
    _pending_commands.clear();
    _next_entity_id = next_id;
    _next_command_sequence = 0U;
    _tick = snapshot.tick.value;
    return FT_ERR_SUCCESS;
}

uint64_t DeterministicWorld::canonical_state_hash() const noexcept
{
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xffU;
            hash *= 1099511628211ULL;
        }
    };
    mix(_tick);
    for (const EntityState &entity : _entities)
    {
        mix(entity.id.value);
        mix(static_cast<uint64_t>(entity.value));
        mix(static_cast<uint64_t>(entity.alive));
    }
    return hash;
}

DeterministicWorld::EntityState *DeterministicWorld::find_entity(EntityId entity_id) noexcept
{
    for (EntityState &entity : _entities)
        if (entity.id.value == entity_id.value)
            return &entity;
    return nullptr;
}

const DeterministicWorld::EntityState *DeterministicWorld::find_entity(EntityId entity_id) const noexcept
{
    for (const EntityState &entity : _entities)
        if (entity.id.value == entity_id.value)
            return &entity;
    return nullptr;
}

}
