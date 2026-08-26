#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/World.hpp"

namespace cnc
{

enum class DamageType : uint8_t
{
    generic = 0U,
    physical = 1U,
    explosive = 2U,
    fire = 3U
};

struct HealthState
{
    EntityId entity;
    int64_t current = 0;
    int64_t maximum = 0;
    Bool alive = FT_FALSE;
};

struct DamageRequest
{
    EntityId target;
    int64_t amount = 0;
    DamageType type = DamageType::generic;
    uint64_t sequence = 0U;
};

class CombatRegistry final
{
public:
    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error register_health(EntityId entity, int64_t maximum,
                          int64_t current = -1) noexcept;
    Error remove(EntityId entity) noexcept;
    Error health(EntityId entity, HealthState *health_out) const noexcept;
    Error queue_damage(EntityId target, int64_t amount,
                       DamageType type = DamageType::generic) noexcept;
    Error apply() noexcept;
    Error discard() noexcept;
    uint64_t canonical_state_hash() const noexcept;
    Size pending_count() const noexcept;

private:
    std::vector<HealthState> _health;
    std::vector<DamageRequest> _requests;
    uint64_t _next_sequence = 0U;
    Bool _initialized = FT_FALSE;
};

}
