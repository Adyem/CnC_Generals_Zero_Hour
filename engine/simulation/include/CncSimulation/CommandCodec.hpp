#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/World.hpp"

namespace cnc
{

struct WorldCommandFrame
{
    uint32_t schema_version = 1U;
    SimulationTick tick;
    std::vector<WorldCommand> commands;
};

class WorldCommandCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static constexpr ft_size_t max_commands = static_cast<ft_size_t>(1U << 20U);

    static Error encode(const WorldCommandFrame &frame,
                        std::vector<uint8_t> *bytes_out) noexcept;
    static Error decode(const uint8_t *bytes, ft_size_t byte_count,
                        WorldCommandFrame *frame_out) noexcept;
};

}
