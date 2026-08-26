#include "ZeroHourData/SpecialPowerLedger.hpp"

#include <limits>
#include <algorithm>
#include <utility>

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

cnc::Error SpecialPowerLedger::export_snapshot(Snapshot *out) const noexcept
{
    if (!out) return FT_ERR_INVALID_POINTER;
    if (!_initialized) return FT_ERR_INVALID_STATE;
    try { out->schema_version=1U; out->cooldowns.clear(); out->cooldowns.reserve(_cooldowns.size()); for (const Cooldown &c:_cooldowns) out->cooldowns.push_back({c.power,c.ready_at}); std::sort(out->cooldowns.begin(),out->cooldowns.end(),[](const SnapshotEntry&a,const SnapshotEntry&b) noexcept{return a.power.value<b.power.value;}); }
    catch (...) { out->cooldowns.clear(); return FT_ERR_NO_MEMORY; } return FT_ERR_SUCCESS;
}
cnc::Error SpecialPowerLedger::import_snapshot(const Snapshot &s) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (s.schema_version!=1U||s.cooldowns.size()>(1U<<20U)) return FT_ERR_CONFIGURATION;
    std::vector<Cooldown> restored; try { restored.reserve(s.cooldowns.size()); for(cnc::Size i=0;i<s.cooldowns.size();++i){const SnapshotEntry&e=s.cooldowns[i]; if(e.power.value==0U||_catalog->find_special_power(e.power)==nullptr||(i&&s.cooldowns[i-1U].power.value>=e.power.value)) return FT_ERR_CONFIGURATION; restored.push_back({e.power,e.ready_at});} } catch (...) { return FT_ERR_NO_MEMORY; } _cooldowns.swap(restored); return FT_ERR_SUCCESS;
}
void SpecialPowerLedger::swap(SpecialPowerLedger &other) noexcept { _cooldowns.swap(other._cooldowns); std::swap(_catalog,other._catalog); std::swap(_initialized,other._initialized); }

cnc::Error SpecialPowerLedger::shutdown() noexcept
{
    _cooldowns.clear();
    _catalog = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
