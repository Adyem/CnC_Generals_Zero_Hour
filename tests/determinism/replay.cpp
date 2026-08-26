#include <cstdint>
#include <vector>

#include "CncGame/GameSession.hpp"

namespace
{
bool run_stream(const std::vector<int64_t> &deltas,
                std::vector<uint64_t> *hashes) noexcept
{
    cnc::GameSession session;
    if (session.initialize() != FT_ERR_SUCCESS ||
        session.install_default_data() != FT_ERR_SUCCESS)
        return false;
    cnc::EntityId entity;
    if (session.players().create_player(cnc::PlayerId{1U}) != FT_ERR_SUCCESS ||
        session.world().create_entity(&entity) != FT_ERR_SUCCESS ||
        session.players().set_owner(entity, cnc::PlayerId{1U}) != FT_ERR_SUCCESS)
        return false;
    for (const int64_t delta : deltas)
    {
        if (session.submit_world_delta(entity, delta) != FT_ERR_SUCCESS ||
            session.advance_one_tick() != FT_ERR_SUCCESS)
            return false;
        hashes->push_back(session.canonical_state_hash());
    }
    return session.shutdown() == FT_ERR_SUCCESS;
}
}

int main()
{
    const std::vector<int64_t> stream{3, -1, 8, 0, 5, -4};
    std::vector<uint64_t> first;
    std::vector<uint64_t> second;
    if (!run_stream(stream, &first) || !run_stream(stream, &second))
        return 1;
    if (first != second || first.empty())
        return 2;
    return 0;
}
