#pragma once

#include <cstdint>
#include <vector>

#include "CncRuntime/Types.hpp"
#include "CncSimulation/World.hpp"

namespace cnc
{

struct PlayerId
{
    uint32_t value = 0U;
    bool is_valid() const noexcept { return value != 0U; }
};

enum class Diplomacy : uint8_t
{
    neutral = 0U,
    allied = 1U,
    hostile = 2U
};

// Generic player/relationship state. Game modules own faction, commander,
// economy, and victory rules; the engine owns identity and deterministic links.
class PlayerRegistry final
{
public:
    Error initialize() noexcept;
    Error create_player(PlayerId id) noexcept;
    Error remove_player(PlayerId id) noexcept;
    Error set_relationship(PlayerId first, PlayerId second,
                           Diplomacy relationship) noexcept;
    Error relationship(PlayerId first, PlayerId second,
                       Diplomacy *relationship_out) const noexcept;
    Error set_owner(EntityId entity, PlayerId owner) noexcept;
    Error clear_owner(EntityId entity) noexcept;
    Error owner(EntityId entity, PlayerId *owner_out) const noexcept;
    Bool contains(PlayerId id) const noexcept;
    Size player_count() const noexcept;
    Error shutdown() noexcept;

private:
    struct RelationshipEntry
    {
        PlayerId first;
        PlayerId second;
        Diplomacy value = Diplomacy::neutral;
    };
    struct OwnershipEntry
    {
        EntityId entity;
        PlayerId owner;
    };
    std::vector<PlayerId> _players;
    std::vector<RelationshipEntry> _relationships;
    std::vector<OwnershipEntry> _ownership;
    Bool _initialized = FT_FALSE;

    RelationshipEntry *find_relationship(PlayerId first, PlayerId second) noexcept;
    const RelationshipEntry *find_relationship(PlayerId first, PlayerId second) const noexcept;
};

}
