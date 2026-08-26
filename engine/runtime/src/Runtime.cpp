#include "CncRuntime/Runtime.hpp"

#include <chrono>

namespace cnc
{

Error Runtime::initialize() noexcept
{
    if (_initialized == FT_TRUE)
        return FT_ERR_ALREADY_INITIALISED;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error Runtime::shutdown() noexcept
{
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error Runtime::set_monotonic_clock(MonotonicClock clock) noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    if (clock == nullptr) return FT_ERR_INVALID_POINTER;
    _monotonic_clock = clock;
    return FT_ERR_SUCCESS;
}

Bool Runtime::is_initialized() const noexcept
{
    return _initialized;
}

const char *Runtime::backend_name() const noexcept { return "runtime-clock"; }

uint64_t Runtime::monotonic_milliseconds() const noexcept
{
    return _monotonic_clock();
}

uint64_t Runtime::default_monotonic_clock() noexcept
{
    const std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();
    const std::chrono::milliseconds elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return static_cast<uint64_t>(elapsed.count());
}

Error Runtime::checked_add_size(Size left, Size right, Size *result) const noexcept
{
    return ft_size_add_checked(left, right, result);
}

}
