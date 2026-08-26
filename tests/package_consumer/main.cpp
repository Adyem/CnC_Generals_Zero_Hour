#include "CncGame/GameSession.hpp"

int main()
{
    cnc::GameSession session;
    if (session.initialize() != FT_ERR_SUCCESS ||
        session.install_default_data() != FT_ERR_SUCCESS)
        return 1;
    return session.shutdown() == FT_ERR_SUCCESS ? 0 : 2;
}
