#include "CncSimulation/LocomotionQueue.hpp"

#include <algorithm>
#include <limits>

#include "errno.hpp"

namespace cnc
{

Error LocomotionQueue::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _requests.clear();
    _next_sequence = 0U;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error LocomotionQueue::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _requests.clear();
    _next_sequence = 0U;
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error LocomotionQueue::queue_move(EntityId entity, WorldCoordinate delta_x,
                                  WorldCoordinate delta_y) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    if (_next_sequence == std::numeric_limits<uint64_t>::max()) return FT_ERR_OUT_OF_RANGE;
    try { _requests.push_back(LocomotionRequest{entity, delta_x, delta_y, _next_sequence++}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error LocomotionQueue::apply(SpatialIndex *spatial_index) noexcept
{
    if (spatial_index == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    SpatialIndexSnapshot projected;
    Error error = spatial_index->export_snapshot(&projected);
    if (error != FT_ERR_SUCCESS) return error;
    try
    {
        std::vector<LocomotionRequest> ordered = _requests;
        std::stable_sort(ordered.begin(), ordered.end(),
                         [](const LocomotionRequest &first, const LocomotionRequest &second)
                         { return first.sequence < second.sequence; });
        for (const LocomotionRequest &request : ordered)
        {
            SpatialPosition *position = nullptr;
            for (SpatialPosition &candidate : projected.entries)
                if (candidate.entity.value == request.entity.value)
                {
                    position = &candidate;
                    break;
                }
            if (position == nullptr) return FT_ERR_NOT_FOUND;
            if ((request.delta_x > 0 && position->x > std::numeric_limits<WorldCoordinate>::max() - request.delta_x) ||
                (request.delta_x < 0 && position->x < std::numeric_limits<WorldCoordinate>::min() - request.delta_x) ||
                (request.delta_y > 0 && position->y > std::numeric_limits<WorldCoordinate>::max() - request.delta_y) ||
                (request.delta_y < 0 && position->y < std::numeric_limits<WorldCoordinate>::min() - request.delta_y))
                return FT_ERR_OUT_OF_RANGE;
            position->x += request.delta_x;
            position->y += request.delta_y;
        }
        std::sort(projected.entries.begin(), projected.entries.end(),
                  [](const SpatialPosition &first, const SpatialPosition &second) noexcept
                  { return first.entity.value < second.entity.value; });
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    error = spatial_index->import_snapshot(projected);
    if (error != FT_ERR_SUCCESS) return error;
    _requests.clear();
    _next_sequence = 0U;
    return FT_ERR_SUCCESS;
}

Error LocomotionQueue::discard() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    _requests.clear();
    _next_sequence = 0U;
    return FT_ERR_SUCCESS;
}

Size LocomotionQueue::pending_count() const noexcept
{
    return static_cast<Size>(_requests.size());
}

}
