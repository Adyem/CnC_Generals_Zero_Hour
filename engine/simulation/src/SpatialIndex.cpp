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
