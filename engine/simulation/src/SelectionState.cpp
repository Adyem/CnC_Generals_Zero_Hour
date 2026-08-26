#include "CncSimulation/SelectionState.hpp"

#include <algorithm>
#include <utility>

#include "errno.hpp"

namespace cnc
{

Error SelectionState::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _groups.clear();
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error SelectionState::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _groups.clear();
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error SelectionState::set_selection(PlayerId player,
                                    const std::vector<EntityId> &entities) noexcept
{
    return set_group(player, 0U, entities);
}

Error SelectionState::selection(PlayerId player,
                                std::vector<EntityId> *entities_out) const noexcept
{
    return control_group(player, 0U, entities_out);
}

Error SelectionState::set_control_group(PlayerId player, uint8_t group,
                                        const std::vector<EntityId> &entities) noexcept
{
    return set_group(player, group, entities);
}

Error SelectionState::control_group(PlayerId player, uint8_t group,
                                    std::vector<EntityId> *entities_out) const noexcept
{
    if (entities_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid() || group >= control_group_count) return FT_ERR_INVALID_ARGUMENT;
    const Group *const found = find_group(player, group);
    try
    {
        entities_out->clear();
        if (found != nullptr) *entities_out = found->entities;
    }
    catch (...)
    {
        entities_out->clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error SelectionState::set_group(PlayerId player, uint8_t group,
                                const std::vector<EntityId> &entities) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid() || group >= control_group_count) return FT_ERR_INVALID_ARGUMENT;
    try
    {
        std::vector<EntityId> ordered = entities;
        std::sort(ordered.begin(), ordered.end(),
                  [](EntityId first, EntityId second) noexcept
                  { return first.value < second.value; });
        for (ft_size_t index = 0U; index < ordered.size(); ++index)
            if (!ordered[index].is_valid() ||
                (index != 0U && ordered[index - 1U].value == ordered[index].value))
                return FT_ERR_INVALID_ARGUMENT;
        std::vector<Group> projected = _groups;
        bool replaced = false;
        for (Group &entry : projected)
            if (entry.player.value == player.value && entry.group == group)
            {
                entry.entities.swap(ordered);
                replaced = true;
                break;
            }
        if (!replaced) projected.push_back(Group{player, group, std::move(ordered)});
        std::sort(projected.begin(), projected.end(),
                  [](const Group &first, const Group &second) noexcept
                  {
                      return first.player.value == second.player.value
                                 ? first.group < second.group
                                 : first.player.value < second.player.value;
                  });
        _groups.swap(projected);
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error SelectionState::clear_player(PlayerId player) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!player.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _groups.begin(); iterator != _groups.end();)
        if (iterator->player.value == player.value) iterator = _groups.erase(iterator);
        else ++iterator;
    return FT_ERR_SUCCESS;
}

Error SelectionState::remove_entity(EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (Group &group : _groups)
        group.entities.erase(std::remove_if(group.entities.begin(), group.entities.end(),
                                            [entity](EntityId candidate)
                                            { return candidate.value == entity.value; }),
                             group.entities.end());
    return FT_ERR_SUCCESS;
}

const SelectionState::Group *SelectionState::find_group(PlayerId player,
                                                         uint8_t group) const noexcept
{
    for (const Group &entry : _groups)
        if (entry.player.value == player.value && entry.group == group) return &entry;
    return nullptr;
}

uint64_t SelectionState::canonical_state_hash() const noexcept
{
    if (_initialized != FT_TRUE) return 0U;
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    for (const Group &group : _groups)
    {
        mix(group.player.value);
        mix(group.group);
        for (const EntityId entity : group.entities) mix(entity.value);
    }
    return hash;
}

}
