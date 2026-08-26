#include "CncNetwork/NetworkSession.hpp"

#include "errno.hpp"

namespace cnc
{

Error OfflineNetworkSession::initialize() noexcept { return FT_ERR_SUCCESS; }

Error OfflineNetworkSession::connect(const char *endpoint) noexcept
{
    (void)endpoint;
    return FT_ERR_INVALID_OPERATION;
}

Error OfflineNetworkSession::send(const uint8_t *payload, Size size) noexcept
{
    (void)payload;
    (void)size;
    return FT_ERR_INVALID_OPERATION;
}

Error OfflineNetworkSession::shutdown() noexcept { return FT_ERR_SUCCESS; }
Bool OfflineNetworkSession::is_initialized() const noexcept { return FT_TRUE; }
Bool OfflineNetworkSession::is_online() const noexcept { return FT_FALSE; }

}
