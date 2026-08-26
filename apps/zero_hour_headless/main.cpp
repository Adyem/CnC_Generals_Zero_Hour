#include <cstdint>
#include <iostream>

#include "CncGame/GameSession.hpp"
#include "CncRender/Renderer.hpp"
#include "CncBuild/BuildInfo.hpp"

int main()
{
    cnc::GameSession session;
    if (session.initialize() != FT_ERR_SUCCESS ||
        session.install_default_data() != FT_ERR_SUCCESS)
        return 1;

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
              << " tick=" << session.world().tick().value
              << " hash=" << session.world().canonical_state_hash()
              << " frame=" << session.renderer().frame_count()
              << " replay_records=" << session.replay_history().size() << "\n";
    return session.shutdown() == FT_ERR_SUCCESS ? 0 : 4;
}
