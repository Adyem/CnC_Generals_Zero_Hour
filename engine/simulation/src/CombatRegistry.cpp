#include "CncSimulation/CombatRegistry.hpp"

#include <algorithm>
#include <limits>

#include "errno.hpp"

namespace cnc
{

Error CombatRegistry::initialize() noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    _health.clear();
    _requests.clear();
    _next_sequence = 0U;
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _health.clear();
    _requests.clear();
    _next_sequence = 0U;
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::register_health(EntityId entity, int64_t maximum,
                                      int64_t current) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid() || maximum <= 0) return FT_ERR_INVALID_ARGUMENT;
    if (current < 0) current = maximum;
    if (current > maximum) return FT_ERR_INVALID_ARGUMENT;
    for (HealthState &state : _health)
        if (state.entity.value == entity.value)
        {
            state.maximum = maximum;
            state.current = current;
            state.alive = current > 0 ? FT_TRUE : FT_FALSE;
            return FT_ERR_SUCCESS;
        }
    try { _health.push_back(HealthState{entity, current, maximum, current > 0 ? FT_TRUE : FT_FALSE}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::remove(EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (auto iterator = _health.begin(); iterator != _health.end(); ++iterator)
        if (iterator->entity.value == entity.value)
        {
            _health.erase(iterator);
            return FT_ERR_SUCCESS;
        }
    return FT_ERR_NOT_FOUND;
}

Error CombatRegistry::health(EntityId entity, HealthState *health_out) const noexcept
{
    if (health_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (const HealthState &state : _health)
        if (state.entity.value == entity.value)
        {
            *health_out = state;
            return FT_ERR_SUCCESS;
        }
    return FT_ERR_NOT_FOUND;
}

Error CombatRegistry::queue_damage(EntityId target, int64_t amount,
                                    DamageType type) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!target.is_valid() || amount <= 0 ||
        static_cast<uint8_t>(type) > static_cast<uint8_t>(DamageType::fire))
        return FT_ERR_INVALID_ARGUMENT;
    if (_next_sequence == std::numeric_limits<uint64_t>::max()) return FT_ERR_OUT_OF_RANGE;
    try { _requests.push_back(DamageRequest{target, amount, type, _next_sequence++}); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::apply() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        std::vector<HealthState> projected = _health;
        std::vector<DamageRequest> ordered = _requests;
        std::stable_sort(ordered.begin(), ordered.end(),
                         [](const DamageRequest &first, const DamageRequest &second)
                         { return first.sequence < second.sequence; });
        for (const DamageRequest &request : ordered)
        {
            HealthState *state = nullptr;
            for (HealthState &candidate : projected)
                if (candidate.entity.value == request.target.value)
                {
                    state = &candidate;
                    break;
                }
            if (state == nullptr) return FT_ERR_NOT_FOUND;
            if (state->alive != FT_TRUE) continue;
            if (request.amount >= state->current)
            {
                state->current = 0;
                state->alive = FT_FALSE;
            }
            else
                state->current -= request.amount;
        }
        _health.swap(projected);
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _requests.clear();
    _next_sequence = 0U;
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::discard() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    _requests.clear();
    _next_sequence = 0U;
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::export_snapshot(CombatRegistrySnapshot *snapshot_out) const noexcept
{
    if (snapshot_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        snapshot_out->schema_version = 1U;
        snapshot_out->health = _health;
        std::sort(snapshot_out->health.begin(), snapshot_out->health.end(),
                  [](const HealthState &first, const HealthState &second) noexcept
                  { return first.entity.value < second.entity.value; });
    }
    catch (...)
    {
        snapshot_out->health.clear();
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error CombatRegistry::import_snapshot(const CombatRegistrySnapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U || snapshot.health.size() > (1U << 20U))
        return FT_ERR_CONFIGURATION;
    std::vector<HealthState> restored;
    try
    {
        restored.reserve(snapshot.health.size());
        for (ft_size_t index = 0U; index < snapshot.health.size(); ++index)
        {
            const HealthState &state = snapshot.health[index];
            if (!state.entity.is_valid() || state.maximum <= 0 || state.current < 0 ||
                state.current > state.maximum ||
                state.alive != (state.current > 0 ? FT_TRUE : FT_FALSE) ||
                (index != 0U && snapshot.health[index - 1U].entity.value >= state.entity.value))
                return FT_ERR_CONFIGURATION;
            restored.push_back(state);
        }
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _health.swap(restored);
    return FT_ERR_SUCCESS;
}

void CombatRegistry::swap(CombatRegistry &other) noexcept
{
    _health.swap(other._health);
    _requests.swap(other._requests);
    const uint64_t sequence = _next_sequence;
    _next_sequence = other._next_sequence;
    other._next_sequence = sequence;
    const Bool initialized = _initialized;
    _initialized = other._initialized;
    other._initialized = initialized;
}

uint64_t CombatRegistry::canonical_state_hash() const noexcept
{
    if (_initialized != FT_TRUE) return 0U;
    std::vector<HealthState> ordered;
    try { ordered = _health; }
    catch (...) { return 0U; }
    std::sort(ordered.begin(), ordered.end(),
              [](const HealthState &first, const HealthState &second) noexcept
              { return first.entity.value < second.entity.value; });
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value)
    {
        for (uint32_t index = 0U; index < 8U; ++index)
        {
            hash ^= (value >> (index * 8U)) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    for (const HealthState &state : ordered)
    {
        mix(state.entity.value);
        mix(static_cast<uint64_t>(state.current));
        mix(static_cast<uint64_t>(state.maximum));
        mix(static_cast<uint64_t>(state.alive));
    }
    return hash;
}

Size CombatRegistry::pending_count() const noexcept
{
    return static_cast<Size>(_requests.size());
}

}
