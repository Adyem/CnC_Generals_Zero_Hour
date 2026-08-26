#pragma once

#include <cstdint>

#include "CncRuntime/Types.hpp"
#include "game_event_scheduler.hpp"

namespace cnc
{

// Optional bridge for the Libft Game scheduler. The default offline build
// keeps SystemRegistry as its deterministic phase scheduler; this adapter is
// enabled only when the complete Libft Game dependency graph is available.
class LibftEventSchedulerAdapter final
{
public:
    LibftEventSchedulerAdapter() noexcept = default;
    LibftEventSchedulerAdapter(const LibftEventSchedulerAdapter &) = delete;
    LibftEventSchedulerAdapter &operator=(const LibftEventSchedulerAdapter &) = delete;

    Error initialize() noexcept;
    Error shutdown() noexcept;
    ft_size_t pending_count() const noexcept;
    const char *backend_name() const noexcept;

private:
    game_event_scheduler _scheduler;
    ft_bool _initialized = FT_FALSE;
};

}
