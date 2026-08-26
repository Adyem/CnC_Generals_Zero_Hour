#include "ZeroHourData/PlayerStateRegistry.hpp"

#include <algorithm>
#include <utility>

#include "errno.hpp"

namespace zero_hour
{

cnc::Error PlayerStateRegistry::initialize(const Catalog *catalog, ScienceLedger *science,
                                           SpecialPowerLedger *powers,
                                           GeneralRoster *generals) noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr || science == nullptr || powers == nullptr || generals == nullptr)
        return FT_ERR_INVALID_POINTER;
    _catalog = catalog;
    _science = science;
    _powers = powers;
    _generals = generals;
    _entries.clear();
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error PlayerStateRegistry::create(cnc::PlayerId player) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (!player.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    if (find(player) != nullptr) return FT_ERR_ALREADY_EXISTS;
    try
    {
        _entries.push_back(Entry{player, PlayerState{}});
        const cnc::Error error = _entries.back().state.initialize(
            _catalog, _science, _powers, _generals);
        if (error != FT_ERR_SUCCESS)
        {
            _entries.pop_back();
            return error;
        }
    }
    catch (...)
    {
        if (!_entries.empty() && _entries.back().player.value == player.value)
            _entries.pop_back();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

cnc::Error PlayerStateRegistry::remove(cnc::PlayerId player) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (!player.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _entries.begin(); iterator != _entries.end(); ++iterator)
        if (iterator->player.value == player.value)
        {
            (void)iterator->state.shutdown();
            _entries.erase(iterator);
            return FT_ERR_SUCCESS;
        }
    return FT_ERR_NOT_FOUND;
}

PlayerState *PlayerStateRegistry::find(cnc::PlayerId player) noexcept
{
    for (Entry &entry : _entries)
        if (entry.player.value == player.value) return &entry.state;
    return nullptr;
}

const PlayerState *PlayerStateRegistry::find(cnc::PlayerId player) const noexcept
{
    for (const Entry &entry : _entries)
        if (entry.player.value == player.value) return &entry.state;
    return nullptr;
}

cnc::Size PlayerStateRegistry::size() const noexcept
{
    return static_cast<cnc::Size>(_entries.size());
}

uint64_t PlayerStateRegistry::canonical_state_hash() const noexcept
{
    if (!_initialized) return 0U;
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    std::vector<const Entry *> ordered;
    try
    {
        ordered.reserve(_entries.size());
        for (const Entry &entry : _entries) ordered.push_back(&entry);
        std::sort(ordered.begin(), ordered.end(),
                  [](const Entry *first, const Entry *second) noexcept
                  { return first->player.value < second->player.value; });
    }
    catch (...) { return 0U; }
    for (const Entry *entry : ordered)
    {
        mix(entry->player.value);
        mix(entry->state.faction().value);
        mix(entry->state.science_points());
    }
    return hash;
}

cnc::Error PlayerStateRegistry::export_snapshot(Snapshot *snapshot_out) const noexcept
{
    if (snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (!_initialized) return FT_ERR_INVALID_STATE;
    try
    {
        snapshot_out->schema_version = 1U;
        snapshot_out->entries.clear();
        snapshot_out->entries.reserve(_entries.size());
        for (const Entry &entry : _entries)
            snapshot_out->entries.push_back(SnapshotEntry{
                entry.player, entry.state.faction(), entry.state.science_points()});
        std::sort(snapshot_out->entries.begin(), snapshot_out->entries.end(),
                  [](const SnapshotEntry &first, const SnapshotEntry &second) noexcept
                  { return first.player.value < second.player.value; });
    }
    catch (...)
    {
        snapshot_out->entries.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

cnc::Error PlayerStateRegistry::import_snapshot(const Snapshot &snapshot) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (snapshot.schema_version != 1U || snapshot.entries.size() > (1U << 20U))
        return FT_ERR_CONFIGURATION;
    std::vector<Entry> restored;
    const auto discard = [&restored]() noexcept
    {
        for (Entry &entry : restored) (void)entry.state.shutdown();
    };
    try
    {
        restored.reserve(snapshot.entries.size());
        for (cnc::Size index = 0U; index < snapshot.entries.size(); ++index)
        {
            const SnapshotEntry &record = snapshot.entries[index];
            if (!record.player.is_valid() ||
                (index != 0U && snapshot.entries[index - 1U].player.value >= record.player.value))
                return FT_ERR_CONFIGURATION;
            Entry entry{record.player, PlayerState{}};
            cnc::Error error = entry.state.initialize(_catalog, _science, _powers, _generals);
            if (error != FT_ERR_SUCCESS) { discard(); return error; }
            if (record.faction.value != 0U)
            {
                error = entry.state.set_faction(record.faction);
                if (error != FT_ERR_SUCCESS) { discard(); return error; }
            }
            error = entry.state.set_science_points(record.science_points);
            if (error != FT_ERR_SUCCESS) { discard(); return error; }
            restored.push_back(std::move(entry));
        }
    }
    catch (...)
    {
        discard();
        return FT_ERR_NO_MEMORY;
    }
    for (Entry &entry : _entries) (void)entry.state.shutdown();
    _entries.swap(restored);
    return FT_ERR_SUCCESS;
}

void PlayerStateRegistry::swap(PlayerStateRegistry &other) noexcept
{
    _entries.swap(other._entries);
    const Catalog *catalog = _catalog;
    _catalog = other._catalog;
    other._catalog = catalog;
    ScienceLedger *science = _science;
    _science = other._science;
    other._science = science;
    SpecialPowerLedger *powers = _powers;
    _powers = other._powers;
    other._powers = powers;
    GeneralRoster *generals = _generals;
    _generals = other._generals;
    other._generals = generals;
    const bool initialized = _initialized;
    _initialized = other._initialized;
    other._initialized = initialized;
}

cnc::Error PlayerStateRegistry::shutdown() noexcept
{
    if (!_initialized) return FT_ERR_SUCCESS;
    for (Entry &entry : _entries) (void)entry.state.shutdown();
    _entries.clear();
    _catalog = nullptr;
    _science = nullptr;
    _powers = nullptr;
    _generals = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
