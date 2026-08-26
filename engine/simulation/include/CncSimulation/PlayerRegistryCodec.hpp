#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/PlayerRegistry.hpp"

namespace cnc
{

// Canonical little-endian representation of generic player/team state.
// Game-owned faction and rule data is intentionally not encoded here.
class PlayerRegistryCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static constexpr ft_size_t max_records = static_cast<ft_size_t>(1U << 20U);

    static Error encode(const PlayerRegistrySnapshot &snapshot,
                        std::vector<uint8_t> *bytes_out) noexcept;
    static Error decode(const uint8_t *bytes, ft_size_t byte_count,
                        PlayerRegistrySnapshot *snapshot_out) noexcept;
};

}
