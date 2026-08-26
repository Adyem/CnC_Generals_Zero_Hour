#pragma once

#include <cstdint>

#include "CncRuntime/Types.hpp"

namespace cnc
{

class Runtime final
{
public:
    using MonotonicClock = uint64_t (*)() noexcept;
    Runtime() noexcept = default;
    Runtime(const Runtime &) = delete;
    Runtime &operator=(const Runtime &) = delete;

    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error set_monotonic_clock(MonotonicClock clock) noexcept;

    Bool is_initialized() const noexcept;
    const char *backend_name() const noexcept;
    uint64_t monotonic_milliseconds() const noexcept;

    Error checked_add_size(Size left, Size right, Size *result) const noexcept;

private:
    static uint64_t default_monotonic_clock() noexcept;
    Bool _initialized = FT_FALSE;
    MonotonicClock _monotonic_clock = &Runtime::default_monotonic_clock;
};

}
