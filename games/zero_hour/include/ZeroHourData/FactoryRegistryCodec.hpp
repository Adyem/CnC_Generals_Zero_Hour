#pragma once

#include <cstdint>
#include <vector>

#include "ZeroHourData/FactoryRegistry.hpp"

namespace zero_hour
{

class FactoryRegistryCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static cnc::Error encode(const FactoryRegistry::Snapshot &snapshot,
                             std::vector<uint8_t> *bytes_out) noexcept;
    static cnc::Error decode(const uint8_t *bytes, cnc::Size byte_count,
                             FactoryRegistry::Snapshot *snapshot_out) noexcept;
};

}
