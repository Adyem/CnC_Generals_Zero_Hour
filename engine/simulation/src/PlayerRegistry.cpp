#include "CncSimulation/PlayerRegistry.hpp"

#include "errno.hpp"

namespace cnc
{

Error PlayerRegistry::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _players.clear();
    _relationships.clear();
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Bool PlayerRegistry::contains(PlayerId id) const noexcept
{
    if (!id.is_valid()) return FT_FALSE;
    for (const PlayerId player : _players)
        if (player.value == id.value) return FT_TRUE;
    return FT_FALSE;
}

Error PlayerRegistry::create_player(PlayerId id) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!id.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    if (contains(id) == FT_TRUE) return FT_ERR_ALREADY_EXISTS;
    try { _players.push_back(id); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::remove_player(PlayerId id) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (contains(id) != FT_TRUE) return FT_ERR_NOT_FOUND;
    for (auto iterator = _players.begin(); iterator != _players.end(); ++iterator)
    {
        if (iterator->value == id.value)
        {
            _players.erase(iterator);
            break;
        }
    }
    for (auto iterator = _relationships.begin(); iterator != _relationships.end();)
    {
        if (iterator->first.value == id.value || iterator->second.value == id.value)
            iterator = _relationships.erase(iterator);
        else
            ++iterator;
    }
    return FT_ERR_SUCCESS;
}

PlayerRegistry::RelationshipEntry *PlayerRegistry::find_relationship(
    PlayerId first, PlayerId second) noexcept
{
    for (RelationshipEntry &entry : _relationships)
        if (entry.first.value == first.value && entry.second.value == second.value)
            return &entry;
    return nullptr;
}

const PlayerRegistry::RelationshipEntry *PlayerRegistry::find_relationship(
    PlayerId first, PlayerId second) const noexcept
{
    for (const RelationshipEntry &entry : _relationships)
        if (entry.first.value == first.value && entry.second.value == second.value)
            return &entry;
    return nullptr;
}

Error PlayerRegistry::set_relationship(PlayerId first, PlayerId second,
                                        Diplomacy value) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (contains(first) != FT_TRUE || contains(second) != FT_TRUE)
        return FT_ERR_NOT_FOUND;
    if (value != Diplomacy::neutral && value != Diplomacy::allied &&
        value != Diplomacy::hostile)
        return FT_ERR_INVALID_ARGUMENT;
    if (first.value == second.value)
        return value == Diplomacy::allied ? FT_ERR_SUCCESS : FT_ERR_INVALID_ARGUMENT;
    try
    {
        std::vector<RelationshipEntry> projected = _relationships;
        RelationshipEntry *entry = nullptr;
        RelationshipEntry *reverse = nullptr;
        for (RelationshipEntry &candidate : projected)
        {
            if (candidate.first.value == first.value && candidate.second.value == second.value)
                entry = &candidate;
            if (candidate.first.value == second.value && candidate.second.value == first.value)
                reverse = &candidate;
        }
        if (entry == nullptr) projected.push_back(RelationshipEntry{first, second, value});
        else entry->value = value;
        if (reverse == nullptr) projected.push_back(RelationshipEntry{second, first, value});
        else reverse->value = value;
        _relationships.swap(projected);
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::relationship(PlayerId first, PlayerId second,
                                    Diplomacy *relationship_out) const noexcept
{
    if (relationship_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (contains(first) != FT_TRUE || contains(second) != FT_TRUE)
        return FT_ERR_NOT_FOUND;
    if (first.value == second.value)
    {
        *relationship_out = Diplomacy::allied;
        return FT_ERR_SUCCESS;
    }
    const RelationshipEntry *entry = find_relationship(first, second);
    *relationship_out = entry == nullptr ? Diplomacy::neutral : entry->value;
    return FT_ERR_SUCCESS;
}

Size PlayerRegistry::player_count() const noexcept
{
    return static_cast<Size>(_players.size());
}

Error PlayerRegistry::shutdown() noexcept
{
    _relationships.clear();
    _players.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}
}
