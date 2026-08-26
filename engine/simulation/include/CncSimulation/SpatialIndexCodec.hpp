#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/SpatialIndex.hpp"

namespace cnc
{

class SpatialIndexCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static constexpr ft_size_t max_entries = static_cast<ft_size_t>(1U << 20U);

    static Error encode(const SpatialIndexSnapshot &snapshot,
                        std::vector<uint8_t> *bytes_out) noexcept;
    static Error decode(const uint8_t *bytes, ft_size_t byte_count,
                        SpatialIndexSnapshot *snapshot_out) noexcept;
};

}
