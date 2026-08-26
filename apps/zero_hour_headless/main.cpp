#include <cstdint>
#include <iostream>
#include <string>

#include "CncGame/GameSession.hpp"
#include "CncRender/Renderer.hpp"
#include "CncBuild/BuildInfo.hpp"

int main(int argc, char **argv)
{
    if (argc == 2 && std::string(argv[1]) == "--help")
    {
        std::cout << "usage: cnc_zero_hour_headless [--manifest <path>]\n"
                  << "backend: libft-basic + deterministic offline core\n"
                  << "renderer: headless; multiplayer: disabled\n";
        return 0;
    }
    cnc::GameSession session;
    if (session.initialize() != FT_ERR_SUCCESS)
        return 1;

    cnc::Error data_error = FT_ERR_SUCCESS;
    if (argc == 1)
        data_error = session.install_default_data();
    else if (argc == 3 && std::string(argv[1]) == "--manifest")
        data_error = session.load_data_manifest(argv[2]);
    else
        data_error = FT_ERR_INVALID_ARGUMENT;
    if (data_error != FT_ERR_SUCCESS)
    {
        (void)session.shutdown();
        return 1;
    }

    cnc::EntityId entity;
    if (session.world().create_entity(&entity) != FT_ERR_SUCCESS ||
        session.submit_world_delta(entity, 1) != FT_ERR_SUCCESS ||
        session.advance_one_tick() != FT_ERR_SUCCESS)
        return 2;

    if (session.renderer().begin_frame() != FT_ERR_SUCCESS ||
        session.renderer().submit(cnc::RenderCommand{1U, 0, 0, 1U, 1U}) != FT_ERR_SUCCESS ||
        session.renderer().end_frame() != FT_ERR_SUCCESS)
        return 3;

    std::cout << "zero_hour_headless version=" << CNC_BUILD_PROJECT_VERSION
              << " commit=" << CNC_BUILD_GIT_COMMIT
              << " runtime=" << session.runtime().backend_name()
              << " world=" << session.world().backend_name()
              << " renderer=" << session.renderer().backend_name()
              << " network=" << session.network().backend_name()
              << " tick=" << session.world().tick().value
              << " hash=" << session.world().canonical_state_hash()
              << " frame=" << session.renderer().frame_count()
              << " replay_records=" << session.replay_history().size() << "\n";
    return session.shutdown() == FT_ERR_SUCCESS ? 0 : 4;
}
