#include "CncSimulation/SpatialIndex.hpp"

#include <algorithm>

#include "errno.hpp"

namespace cnc
{

Error SpatialIndex::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _entries.clear();
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error SpatialIndex::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _entries.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error SpatialIndex::set_position(EntityId entity, WorldCoordinate x,
                                  WorldCoordinate y, uint32_t layer) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (SpatialPosition &entry : _entries)
    {
        if (entry.entity.value == entity.value)
        {
            entry.x = x;
            entry.y = y;
            entry.layer = layer;
            return FT_ERR_SUCCESS;
        }
    }
    try { _entries.push_back(SpatialPosition{entity, x, y, layer}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error SpatialIndex::remove(EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _entries.begin(); iterator != _entries.end(); ++iterator)
    {
        if (iterator->entity.value == entity.value)
        {
            _entries.erase(iterator);
            return FT_ERR_SUCCESS;
        }
    }
    return FT_ERR_NOT_FOUND;
}

Error SpatialIndex::query_box(WorldCoordinate min_x, WorldCoordinate min_y,
                              WorldCoordinate max_x, WorldCoordinate max_y,
                              uint32_t layer, std::vector<EntityId> *entities_out) const noexcept
{
    if (entities_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (min_x > max_x || min_y > max_y) return FT_ERR_INVALID_ARGUMENT;
    try
    {
        entities_out->clear();
        for (const SpatialPosition &entry : _entries)
            if (entry.layer == layer && entry.x >= min_x && entry.x <= max_x &&
                entry.y >= min_y && entry.y <= max_y)
                entities_out->push_back(entry.entity);
        std::sort(entities_out->begin(), entities_out->end(),
                  [](EntityId first, EntityId second) noexcept
                  { return first.value < second.value; });
    }
    catch (...)
    {
        entities_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error SpatialIndex::position(EntityId entity, SpatialPosition *position_out) const noexcept
{
    if (position_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (const SpatialPosition &entry : _entries)
        if (entry.entity.value == entity.value)
        {
            *position_out = entry;
            return FT_ERR_SUCCESS;
        }
    return FT_ERR_NOT_FOUND;
}

Error SpatialIndex::export_snapshot(SpatialIndexSnapshot *snapshot_out) const noexcept
{
    if (snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        snapshot_out->schema_version = 1U;
        snapshot_out->entries = _entries;
        std::sort(snapshot_out->entries.begin(), snapshot_out->entries.end(),
                  [](const SpatialPosition &first, const SpatialPosition &second) noexcept
                  { return first.entity.value < second.entity.value; });
    }
    catch (...)
    {
        snapshot_out->entries.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error SpatialIndex::import_snapshot(const SpatialIndexSnapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U || snapshot.entries.size() > (1U << 20U))
        return FT_ERR_CONFIGURATION;
    std::vector<SpatialPosition> restored;
    try
    {
        restored.reserve(snapshot.entries.size());
        for (ft_size_t index = 0U; index < snapshot.entries.size(); ++index)
        {
            const SpatialPosition &entry = snapshot.entries[index];
            if (!entry.entity.is_valid() ||
                (index != 0U && snapshot.entries[index - 1U].entity.value >= entry.entity.value))
                return FT_ERR_CONFIGURATION;
            restored.push_back(entry);
        }
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _entries.swap(restored);
    return FT_ERR_SUCCESS;
}

void SpatialIndex::swap(SpatialIndex &other) noexcept
{
    _entries.swap(other._entries);
    const Bool initialized = _initialized;
    _initialized = other._initialized;
    other._initialized = initialized;
}

uint64_t SpatialIndex::canonical_state_hash() const noexcept
{
    if (_initialized != FT_TRUE) return 0U;
    std::vector<SpatialPosition> ordered;
    try { ordered = _entries; }
    catch (...) { return 0U; }
    std::sort(ordered.begin(), ordered.end(),
              [](const SpatialPosition &first, const SpatialPosition &second) noexcept
              { return first.entity.value < second.entity.value; });
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    for (const SpatialPosition &entry : ordered)
    {
        mix(entry.entity.value);
        mix(static_cast<uint64_t>(entry.x));
        mix(static_cast<uint64_t>(entry.y));
        mix(entry.layer);
    }
    return hash;
}

Size SpatialIndex::size() const noexcept
{
    return static_cast<Size>(_entries.size());
}

}
