#include "ZeroHourData/PlayerStateRegistry.hpp"

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
