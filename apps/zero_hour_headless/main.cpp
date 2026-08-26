#include <cstdint>
#include <iostream>

#include "CncGame/GameSession.hpp"
#include "CncRender/Renderer.hpp"

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

    cnc::HeadlessRenderer renderer;
    if (renderer.initialize() != FT_ERR_SUCCESS ||
        renderer.begin_frame() != FT_ERR_SUCCESS ||
        renderer.submit(cnc::RenderCommand{1U, 0, 0, 1U, 1U}) != FT_ERR_SUCCESS ||
        renderer.end_frame() != FT_ERR_SUCCESS)
        return 3;

    std::cout << "zero_hour_headless tick=" << session.world().tick().value
              << " hash=" << session.world().canonical_state_hash()
              << " frame=" << renderer.frame_count() << "\n";
    (void)renderer.shutdown();
    return session.shutdown() == FT_ERR_SUCCESS ? 0 : 4;
}
