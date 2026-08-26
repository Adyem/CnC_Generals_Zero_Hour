#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/PlayerRegistry.hpp"

namespace cnc
{

class SelectionState final
{
public:
    static constexpr uint8_t control_group_count = 10U;

    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error set_selection(PlayerId player, const std::vector<EntityId> &entities) noexcept;
    Error selection(PlayerId player, std::vector<EntityId> *entities_out) const noexcept;
    Error set_control_group(PlayerId player, uint8_t group,
                            const std::vector<EntityId> &entities) noexcept;
    Error control_group(PlayerId player, uint8_t group,
                        std::vector<EntityId> *entities_out) const noexcept;
    Error clear_player(PlayerId player) noexcept;
    Error remove_entity(EntityId entity) noexcept;
    uint64_t canonical_state_hash() const noexcept;

private:
    struct Group
    {
        PlayerId player;
        uint8_t group = 0U;
        std::vector<EntityId> entities;
    };
    std::vector<Group> _groups;
    Bool _initialized = FT_FALSE;

    Error set_group(PlayerId player, uint8_t group,
                    const std::vector<EntityId> &entities) noexcept;
    const Group *find_group(PlayerId player, uint8_t group) const noexcept;
};

}
