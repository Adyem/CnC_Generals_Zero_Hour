#include "CncGame/LibftEventSchedulerAdapter.hpp"

namespace cnc
{

Error LibftEventSchedulerAdapter::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    const Error error = _scheduler.initialize();
    if (error != FT_ERR_SUCCESS) return error;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error LibftEventSchedulerAdapter::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    const Error error = _scheduler.destroy();
    _initialized = FT_FALSE;
    return error;
}

ft_size_t LibftEventSchedulerAdapter::pending_count() const noexcept
{
    return _initialized == FT_TRUE ? _scheduler.size() : 0U;
}

Error LibftEventSchedulerAdapter::snapshot_profile(
    t_event_scheduler_profile *profile_out) const noexcept
{
    if (profile_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    _scheduler.snapshot_profile(*profile_out);
    return FT_ERR_SUCCESS;
}

const char *LibftEventSchedulerAdapter::backend_name() const noexcept
{
    return "libft::game_event_scheduler";
}

}
