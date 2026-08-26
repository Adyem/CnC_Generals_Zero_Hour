#include "CncSimulation/PlayerRegistry.hpp"

#include "errno.hpp"

namespace cnc
{

Error PlayerRegistry::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _players.clear();
    _teams.clear();
    _team_memberships.clear();
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
    for (auto iterator = _team_memberships.begin(); iterator != _team_memberships.end();)
    {
        if (iterator->player.value == id.value)
            iterator = _team_memberships.erase(iterator);
        else
            ++iterator;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::create_team(TeamId id) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!id.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (const TeamId team : _teams)
        if (team.value == id.value) return FT_ERR_ALREADY_EXISTS;
    try { _teams.push_back(id); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::remove_team(TeamId id) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!id.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    bool found = false;
    for (auto iterator = _teams.begin(); iterator != _teams.end(); ++iterator)
    {
        if (iterator->value == id.value)
        {
            _teams.erase(iterator);
            found = true;
            break;
        }
    }
    if (!found) return FT_ERR_NOT_FOUND;
    for (auto iterator = _team_memberships.begin(); iterator != _team_memberships.end();)
    {
        if (iterator->team.value == id.value)
            iterator = _team_memberships.erase(iterator);
        else
            ++iterator;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::assign_team(PlayerId player, TeamId team) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid() || !team.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    if (contains(player) != FT_TRUE) return FT_ERR_NOT_FOUND;
    bool team_exists = false;
    for (const TeamId candidate : _teams)
        if (candidate.value == team.value) { team_exists = true; break; }
    if (!team_exists) return FT_ERR_NOT_FOUND;
    for (TeamMembership &membership : _team_memberships)
    {
        if (membership.player.value == player.value)
        {
            membership.team = team;
            return FT_ERR_SUCCESS;
        }
    }
    try { _team_memberships.push_back(TeamMembership{player, team}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::team_of(PlayerId player, TeamId *team_out) const noexcept
{
    if (team_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid() || contains(player) != FT_TRUE) return FT_ERR_NOT_FOUND;
    for (const TeamMembership &membership : _team_memberships)
    {
        if (membership.player.value == player.value)
        {
            *team_out = membership.team;
            return FT_ERR_SUCCESS;
        }
    }
    return FT_ERR_NOT_FOUND;
}

Error PlayerRegistry::are_teammates(PlayerId first, PlayerId second,
                                     Bool *result_out) const noexcept
{
    if (result_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!first.is_valid() || !second.is_valid() || contains(first) != FT_TRUE ||
        contains(second) != FT_TRUE)
        return FT_ERR_NOT_FOUND;
    if (first.value == second.value)
    {
        *result_out = FT_TRUE;
        return FT_ERR_SUCCESS;
    }
    TeamId first_team;
    TeamId second_team;
    const Error first_error = team_of(first, &first_team);
    const Error second_error = team_of(second, &second_team);
    *result_out = (first_error == FT_ERR_SUCCESS && second_error == FT_ERR_SUCCESS &&
                   first_team.value == second_team.value) ? FT_TRUE : FT_FALSE;
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

Error PlayerRegistry::owned_entities(PlayerId owner_id,
                                     std::vector<EntityId> *entities_out) const noexcept
{
    if (entities_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!owner_id.is_valid() || contains(owner_id) != FT_TRUE) return FT_ERR_NOT_FOUND;
    try
    {
        entities_out->clear();
        for (const OwnershipEntry &entry : _ownership)
            if (entry.owner.value == owner_id.value) entities_out->push_back(entry.entity);
    }
    catch (...)
    {
        entities_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::export_snapshot(PlayerRegistrySnapshot *snapshot_out) const noexcept
{
    if (snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        snapshot_out->schema_version = 1U;
        snapshot_out->players = _players;
        snapshot_out->teams = _teams;
        snapshot_out->team_memberships = _team_memberships;
        snapshot_out->relationships = _relationships;
        snapshot_out->ownership = _ownership;
    }
    catch (...)
    {
        snapshot_out->players.clear();
        snapshot_out->teams.clear();
        snapshot_out->team_memberships.clear();
        snapshot_out->relationships.clear();
        snapshot_out->ownership.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error PlayerRegistry::import_snapshot(const PlayerRegistrySnapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U) return FT_ERR_CONFIGURATION;
    try
    {
        const auto has_player = [&snapshot](PlayerId id) noexcept
        {
            for (const PlayerId player : snapshot.players)
                if (player.value == id.value) return true;
            return false;
        };
        for (ft_size_t index = 0U; index < snapshot.players.size(); ++index)
            if (!snapshot.players[index].is_valid() ||
                (index != 0U && snapshot.players[index - 1U].value >= snapshot.players[index].value))
                return FT_ERR_CONFIGURATION;
        for (ft_size_t index = 0U; index < snapshot.teams.size(); ++index)
            if (!snapshot.teams[index].is_valid() ||
                (index != 0U && snapshot.teams[index - 1U].value >= snapshot.teams[index].value))
                return FT_ERR_CONFIGURATION;
        for (const TeamMembership &membership : snapshot.team_memberships)
        {
            if (!membership.player.is_valid() || !membership.team.is_valid() ||
                !has_player(membership.player)) return FT_ERR_CONFIGURATION;
            bool team_exists = false;
            for (const TeamId team : snapshot.teams)
                if (team.value == membership.team.value) { team_exists = true; break; }
            if (!team_exists) return FT_ERR_CONFIGURATION;
        }
        for (ft_size_t index = 0U; index < snapshot.team_memberships.size(); ++index)
            for (ft_size_t next = index + 1U; next < snapshot.team_memberships.size(); ++next)
                if (snapshot.team_memberships[index].player.value ==
                    snapshot.team_memberships[next].player.value)
                    return FT_ERR_CONFIGURATION;
        for (const PlayerRelationship &relationship_value : snapshot.relationships)
        {
            if (!relationship_value.first.is_valid() || !relationship_value.second.is_valid() ||
                relationship_value.first.value == relationship_value.second.value ||
                !has_player(relationship_value.first) || !has_player(relationship_value.second) ||
                (relationship_value.value != Diplomacy::neutral &&
                 relationship_value.value != Diplomacy::allied &&
                 relationship_value.value != Diplomacy::hostile))
                return FT_ERR_CONFIGURATION;
        }
        for (const PlayerOwnership &ownership_value : snapshot.ownership)
            if (!ownership_value.entity.is_valid() || !ownership_value.owner.is_valid() ||
                !has_player(ownership_value.owner)) return FT_ERR_CONFIGURATION;
        for (ft_size_t index = 0U; index < snapshot.ownership.size(); ++index)
            for (ft_size_t next = index + 1U; next < snapshot.ownership.size(); ++next)
                if (snapshot.ownership[index].entity.value == snapshot.ownership[next].entity.value)
                    return FT_ERR_CONFIGURATION;
        for (const PlayerRelationship &relationship_value : snapshot.relationships)
        {
            bool reverse_found = false;
            for (const PlayerRelationship &reverse : snapshot.relationships)
                if (reverse.first.value == relationship_value.second.value &&
                    reverse.second.value == relationship_value.first.value &&
                    reverse.value == relationship_value.value)
                    { reverse_found = true; break; }
            if (!reverse_found) return FT_ERR_CONFIGURATION;
        }
        std::vector<PlayerId> players = snapshot.players;
        std::vector<TeamId> teams = snapshot.teams;
        std::vector<TeamMembership> memberships = snapshot.team_memberships;
        std::vector<RelationshipEntry> relationships = snapshot.relationships;
        std::vector<OwnershipEntry> ownership = snapshot.ownership;
        _players.swap(players);
        _teams.swap(teams);
        _team_memberships.swap(memberships);
        _relationships.swap(relationships);
        _ownership.swap(ownership);
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
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

Error PlayerRegistry::is_allied(PlayerId first, PlayerId second,
                                 Bool *result_out) const noexcept
{
    if (result_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    Diplomacy value = Diplomacy::neutral;
    const Error relationship_error = relationship(first, second, &value);
    if (relationship_error != FT_ERR_SUCCESS) return relationship_error;
    if (value == Diplomacy::allied)
    {
        *result_out = FT_TRUE;
        return FT_ERR_SUCCESS;
    }
    return are_teammates(first, second, result_out);
}

Size PlayerRegistry::player_count() const noexcept
{
    return static_cast<Size>(_players.size());
}

Error PlayerRegistry::shutdown() noexcept
{
    _relationships.clear();
    _teams.clear();
    _team_memberships.clear();
    _ownership.clear();
    _players.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}
}
