#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/World.hpp"

namespace cnc
{

// Canonical little-endian wire representation for offline saves/replays.
// The codec is transport- and filesystem-neutral; callers own the byte buffer.
class WorldSnapshotCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static constexpr ft_size_t max_entries = static_cast<ft_size_t>(1U << 20U);

    static Error encode(const WorldSnapshot &snapshot,
                        std::vector<uint8_t> *bytes_out) noexcept;
    static Error decode(const uint8_t *bytes, ft_size_t byte_count,
                        WorldSnapshot *snapshot_out) noexcept;
};

}
