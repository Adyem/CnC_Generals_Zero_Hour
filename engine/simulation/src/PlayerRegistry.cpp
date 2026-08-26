#include "CncSimulation/PlayerRegistry.hpp"

#include "errno.hpp"

namespace cnc
{

Error PlayerRegistry::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _players.clear();
    _relationships.clear();
    _ownership.clear();
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
    for (auto iterator = _ownership.begin(); iterator != _ownership.end();)
    {
        if (iterator->owner.value == id.value)
            iterator = _ownership.erase(iterator);
        else
            ++iterator;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::set_owner(EntityId entity, PlayerId owner_id) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid() || !owner_id.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    if (contains(owner_id) != FT_TRUE) return FT_ERR_NOT_FOUND;
    for (OwnershipEntry &entry : _ownership)
    {
        if (entry.entity.value == entity.value)
        {
            entry.owner = owner_id;
            return FT_ERR_SUCCESS;
        }
    }
    try { _ownership.push_back(OwnershipEntry{entity, owner_id}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::clear_owner(EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _ownership.begin(); iterator != _ownership.end(); ++iterator)
    {
        if (iterator->entity.value == entity.value)
        {
            _ownership.erase(iterator);
            return FT_ERR_SUCCESS;
        }
    }
    return FT_ERR_NOT_FOUND;
}

Error PlayerRegistry::owner(EntityId entity, PlayerId *owner_out) const noexcept
{
    if (owner_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (const OwnershipEntry &entry : _ownership)
    {
        if (entry.entity.value == entity.value)
        {
            *owner_out = entry.owner;
            return FT_ERR_SUCCESS;
        }
    }
    return FT_ERR_NOT_FOUND;
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
        ft_size_t entry_index = 0U;
        ft_size_t reverse_index = 0U;
        bool has_entry = false;
        bool has_reverse = false;
        for (ft_size_t index = 0U; index < projected.size(); ++index)
        {
            const RelationshipEntry &candidate = projected[index];
            if (candidate.first.value == first.value && candidate.second.value == second.value)
            {
                entry_index = index;
                has_entry = true;
            }
            if (candidate.first.value == second.value && candidate.second.value == first.value)
            {
                reverse_index = index;
                has_reverse = true;
            }
        }
        if (!has_entry) projected.push_back(RelationshipEntry{first, second, value});
        else projected[entry_index].value = value;
        if (!has_reverse) projected.push_back(RelationshipEntry{second, first, value});
        else projected[reverse_index].value = value;
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
    _ownership.clear();
    _players.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}
}
