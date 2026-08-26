#pragma once

#include <cstdint>

#include "CncRuntime/Types.hpp"

namespace cnc
{

class Runtime final
{
public:
    Runtime() noexcept = default;
    Runtime(const Runtime &) = delete;
    Runtime &operator=(const Runtime &) = delete;

    Error initialize() noexcept;
    Error shutdown() noexcept;

    Bool is_initialized() const noexcept;
    uint64_t monotonic_milliseconds() const noexcept;

    Error checked_add_size(Size left, Size right, Size *result) const noexcept;

private:
    Bool _initialized = FT_FALSE;
};

}
