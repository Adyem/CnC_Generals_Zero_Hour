#include "ZeroHourData/SpecialPowerLedger.hpp"

#include <limits>

#include "errno.hpp"

namespace zero_hour
{

cnc::Error SpecialPowerLedger::initialize(const Catalog *catalog) noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr) return FT_ERR_INVALID_POINTER;
    _catalog = catalog;
    _cooldowns.clear();
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error SpecialPowerLedger::activate(cnc::DefinitionId power,
                                         cnc::SimulationTick now,
                                         cnc::SimulationTick *ready_at) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (ready_at == nullptr) return FT_ERR_INVALID_POINTER;
    const auto *definition = _catalog->find_special_power(power);
    if (definition == nullptr) return FT_ERR_NOT_FOUND;
    for (auto &cooldown : _cooldowns)
    {
        if (cooldown.power.value != power.value) continue;
        if (now.value < cooldown.ready_at.value) return FT_ERR_INVALID_OPERATION;
        if (definition->recharge_ticks >
            std::numeric_limits<uint64_t>::max() - now.value)
            return FT_ERR_OUT_OF_RANGE;
        cooldown.ready_at.value = now.value + definition->recharge_ticks;
        *ready_at = cooldown.ready_at;
        return FT_ERR_SUCCESS;
    }
    if (definition->recharge_ticks >
        std::numeric_limits<uint64_t>::max() - now.value)
        return FT_ERR_OUT_OF_RANGE;
    try
    {
        const cnc::SimulationTick ready{now.value + definition->recharge_ticks};
        _cooldowns.push_back(Cooldown{power, ready});
        *ready_at = ready;
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

bool SpecialPowerLedger::is_ready(cnc::DefinitionId power,
                                  cnc::SimulationTick now) const noexcept
{
    for (const auto &cooldown : _cooldowns)
        if (cooldown.power.value == power.value && now.value < cooldown.ready_at.value)
            return false;
    return _catalog != nullptr && _catalog->find_special_power(power) != nullptr;
}

cnc::Size SpecialPowerLedger::active_count() const noexcept
{
    return static_cast<cnc::Size>(_cooldowns.size());
}

cnc::Error SpecialPowerLedger::shutdown() noexcept
{
    _cooldowns.clear();
    _catalog = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
