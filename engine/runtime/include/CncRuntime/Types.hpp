#pragma once

#include <cstdint>

#include "basic.hpp"
#include "errno.hpp"

namespace cnc
{

using Error = int32_t;
using Size = ft_size_t;
using Bool = ft_bool;
using EntityIdValue = uint64_t;
using SimulationTickValue = uint64_t;

struct EntityId
{
    EntityIdValue value = 0U;

    constexpr bool is_valid() const noexcept
    {
        return value != 0U;
    }
};

struct SimulationTick
{
    SimulationTickValue value = 0U;
};

}
