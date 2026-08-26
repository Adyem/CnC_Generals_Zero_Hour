#pragma once

#include <cstdint>
#include <vector>

#include "ZeroHourData/PlayerStateRegistry.hpp"

namespace zero_hour
{

class PlayerStateRegistryCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static constexpr cnc::Size max_entries = static_cast<cnc::Size>(1U << 20U);
    static cnc::Error encode(const PlayerStateRegistry::Snapshot &snapshot,
                             std::vector<uint8_t> *bytes_out) noexcept;
    static cnc::Error decode(const uint8_t *bytes, cnc::Size byte_count,
                             PlayerStateRegistry::Snapshot *snapshot_out) noexcept;
};

}
