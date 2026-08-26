#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/SpatialIndex.hpp"

namespace cnc
{

struct LocomotionRequest
{
    EntityId entity;
    WorldCoordinate delta_x = 0;
    WorldCoordinate delta_y = 0;
    uint64_t sequence = 0U;
};

class LocomotionQueue final
{
public:
    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error queue_move(EntityId entity, WorldCoordinate delta_x,
                     WorldCoordinate delta_y) noexcept;
    Error apply(SpatialIndex *spatial_index) noexcept;
    Error discard() noexcept;
    Size pending_count() const noexcept;

private:
    std::vector<LocomotionRequest> _requests;
    uint64_t _next_sequence = 0U;
    Bool _initialized = FT_FALSE;
};

}
