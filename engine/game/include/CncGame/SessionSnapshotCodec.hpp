#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/PlayerRegistry.hpp"
#include "CncSimulation/SnapshotCodec.hpp"
#include "CncSimulation/SpatialIndex.hpp"
#include "CncSimulation/CombatRegistry.hpp"

namespace cnc
{

struct SessionSnapshot
{
    uint32_t schema_version = 1U;
    WorldSnapshot world;
    PlayerRegistrySnapshot players;
    SpatialIndexSnapshot spatial;
    CombatRegistrySnapshot combat;
};

class SessionSnapshotCodec final
{
public:
    static constexpr uint32_t wire_schema_version = 1U;
    static Error encode(const SessionSnapshot &snapshot,
                        std::vector<uint8_t> *bytes_out) noexcept;
    static Error decode(const uint8_t *bytes, ft_size_t byte_count,
                        SessionSnapshot *snapshot_out) noexcept;
};

}
