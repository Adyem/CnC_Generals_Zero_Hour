#include "CncSimulation/VisibilityRegistry.hpp"

#include <algorithm>

#include "errno.hpp"

namespace cnc
{

Error VisibilityRegistry::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _records.clear();
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _records.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::set_visibility(PlayerId player, EntityId entity,
                                          VisibilityState state) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid() || !entity.is_valid() ||
        static_cast<uint8_t>(state) > static_cast<uint8_t>(VisibilityState::visible))
        return FT_ERR_INVALID_ARGUMENT;
    for (VisibilityRecord &record : _records)
        if (record.player.value == player.value && record.entity.value == entity.value)
        {
            record.state = state;
            return FT_ERR_SUCCESS;
        }
    try { _records.push_back(VisibilityRecord{player, entity, state}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::visibility(PlayerId player, EntityId entity,
                                      VisibilityState *state_out) const noexcept
{
    if (state_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid() || !entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (const VisibilityRecord &record : _records)
        if (record.player.value == player.value && record.entity.value == entity.value)
        {
            *state_out = record.state;
            return FT_ERR_SUCCESS;
        }
    *state_out = VisibilityState::hidden;
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::visible_entities(PlayerId player,
                                            std::vector<EntityId> *entities_out) const noexcept
{
    if (entities_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    try
    {
        entities_out->clear();
        for (const VisibilityRecord &record : _records)
            if (record.player.value == player.value && record.state == VisibilityState::visible)
                entities_out->push_back(record.entity);
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

Error VisibilityRegistry::remove_player(PlayerId player) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _records.begin(); iterator != _records.end();)
        if (iterator->player.value == player.value) iterator = _records.erase(iterator);
        else ++iterator;
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::remove_entity(EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _records.begin(); iterator != _records.end();)
        if (iterator->entity.value == entity.value) iterator = _records.erase(iterator);
        else ++iterator;
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::export_snapshot(VisibilitySnapshot *snapshot_out) const noexcept
{
    if (snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        snapshot_out->schema_version = 1U;
        snapshot_out->records = _records;
        std::sort(snapshot_out->records.begin(), snapshot_out->records.end(),
                  [](const VisibilityRecord &first, const VisibilityRecord &second) noexcept
                  {
                      return first.player.value == second.player.value
                                 ? first.entity.value < second.entity.value
                                 : first.player.value < second.player.value;
                  });
    }
    catch (...)
    {
        snapshot_out->records.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error VisibilityRegistry::import_snapshot(const VisibilitySnapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U || snapshot.records.size() > (1U << 20U))
        return FT_ERR_CONFIGURATION;
    std::vector<VisibilityRecord> restored;
    try
    {
        restored.reserve(snapshot.records.size());
        for (ft_size_t index = 0U; index < snapshot.records.size(); ++index)
        {
            const VisibilityRecord &record = snapshot.records[index];
            if (!record.player.is_valid() || !record.entity.is_valid() ||
                static_cast<uint8_t>(record.state) > static_cast<uint8_t>(VisibilityState::visible) ||
                (index != 0U &&
                 (snapshot.records[index - 1U].player.value > record.player.value ||
                  (snapshot.records[index - 1U].player.value == record.player.value &&
                   snapshot.records[index - 1U].entity.value >= record.entity.value))))
                return FT_ERR_CONFIGURATION;
            restored.push_back(record);
        }
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    _records.swap(restored);
    return FT_ERR_SUCCESS;
}

uint64_t VisibilityRegistry::canonical_state_hash() const noexcept
{
    if (_initialized != FT_TRUE) return 0U;
    VisibilitySnapshot snapshot;
    if (export_snapshot(&snapshot) != FT_ERR_SUCCESS) return 0U;
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    for (const VisibilityRecord &record : snapshot.records)
    {
        mix(record.player.value);
        mix(record.entity.value);
        mix(static_cast<uint64_t>(record.state));
    }
    return hash;
}

Size VisibilityRegistry::size() const noexcept
{
    return static_cast<Size>(_records.size());
}

}
