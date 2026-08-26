#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/World.hpp"

namespace cnc
{

using WorldCoordinate = int64_t;

struct SpatialPosition
{
    EntityId entity;
    WorldCoordinate x = 0;
    WorldCoordinate y = 0;
    uint32_t layer = 0U;
};

class SpatialIndex final
{
public:
    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error set_position(EntityId entity, WorldCoordinate x, WorldCoordinate y,
                       uint32_t layer = 0U) noexcept;
    Error remove(EntityId entity) noexcept;
    Error query_box(WorldCoordinate min_x, WorldCoordinate min_y,
                    WorldCoordinate max_x, WorldCoordinate max_y,
                    uint32_t layer, std::vector<EntityId> *entities_out) const noexcept;
    Error position(EntityId entity, SpatialPosition *position_out) const noexcept;
    uint64_t canonical_state_hash() const noexcept;
    Size size() const noexcept;

private:
    std::vector<SpatialPosition> _entries;
    Bool _initialized = FT_FALSE;
};

}
