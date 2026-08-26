#pragma once

#include <cstdint>

#include "CncRuntime/Types.hpp"

namespace cnc
{

// Multiplayer is intentionally unavailable during the offline migration.
// Keeping this capability explicit prevents UI/game code from assuming that a
// transport exists merely because a network module is present.
class OfflineNetworkSession final
{
public:
    Error initialize() noexcept;
    Error connect(const char *endpoint) noexcept;
    Error send(const uint8_t *payload, Size size) noexcept;
    Error shutdown() noexcept;
    Bool is_initialized() const noexcept;
    Bool is_online() const noexcept;
};

}
