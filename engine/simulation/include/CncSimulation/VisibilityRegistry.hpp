#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/PlayerRegistry.hpp"

namespace cnc
{

enum class VisibilityState : uint8_t
{
    hidden = 0U,
    explored = 1U,
    visible = 2U
};

struct VisibilityRecord
{
    PlayerId player;
    EntityId entity;
    VisibilityState state = VisibilityState::hidden;
};

struct VisibilitySnapshot
{
    uint32_t schema_version = 1U;
    std::vector<VisibilityRecord> records;
};

class VisibilityRegistry final
{
public:
    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error set_visibility(PlayerId player, EntityId entity,
                         VisibilityState state) noexcept;
    Error visibility(PlayerId player, EntityId entity,
                     VisibilityState *state_out) const noexcept;
    Error visible_entities(PlayerId player,
                           std::vector<EntityId> *entities_out) const noexcept;
    Error remove_player(PlayerId player) noexcept;
    Error remove_entity(EntityId entity) noexcept;
    Error export_snapshot(VisibilitySnapshot *snapshot_out) const noexcept;
    Error import_snapshot(const VisibilitySnapshot &snapshot) noexcept;
    uint64_t canonical_state_hash() const noexcept;
    Size size() const noexcept;

private:
    std::vector<VisibilityRecord> _records;
    Bool _initialized = FT_FALSE;
};

}
