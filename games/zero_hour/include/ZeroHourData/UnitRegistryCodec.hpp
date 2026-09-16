#pragma once
#include <cstdint>
#include <vector>
#include "ZeroHourData/UnitRegistry.hpp"
namespace zero_hour
{
class UnitRegistryCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static cnc::Error encode(const UnitRegistry::Snapshot &, std::vector<uint8_t> *) noexcept;
    static cnc::Error decode(const uint8_t *, cnc::Size, UnitRegistry::Snapshot *) noexcept;
};
}
