#pragma once

#include <cstdint>

#include "CncRuntime/Types.hpp"

namespace cnc
{

// Backend-neutral contract. A future Libft transport can replace the offline
// implementation without changing GameSession or game rules.
class NetworkSession
{
public:
    virtual ~NetworkSession() noexcept = default;
    virtual Error initialize() noexcept = 0;
    virtual const char *backend_name() const noexcept = 0;
    virtual Error connect(const char *endpoint) noexcept = 0;
    virtual Error send(const uint8_t *payload, Size size) noexcept = 0;
    virtual Error shutdown() noexcept = 0;
    virtual Bool is_initialized() const noexcept = 0;
    virtual Bool is_online() const noexcept = 0;
};

// Multiplayer is intentionally unavailable during the offline migration.
class OfflineNetworkSession final : public NetworkSession
{
public:
    Error initialize() noexcept override;
    const char *backend_name() const noexcept override;
    Error connect(const char *endpoint) noexcept override;
    Error send(const uint8_t *payload, Size size) noexcept override;
    Error shutdown() noexcept override;
    Bool is_initialized() const noexcept override;
    Bool is_online() const noexcept override;
};

}
