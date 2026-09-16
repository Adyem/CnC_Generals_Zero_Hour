#include "ZeroHourData/UnitRegistry.hpp"

#include <algorithm>

#include "errno.hpp"

namespace zero_hour
{
cnc::Error UnitRegistry::initialize(const Catalog *catalog) noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr) return FT_ERR_INVALID_POINTER;
    _catalog = catalog; _bindings.clear(); _initialized = FT_TRUE; return FT_ERR_SUCCESS;
}
cnc::Error UnitRegistry::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _bindings.clear(); _catalog = nullptr; _initialized = FT_FALSE; return FT_ERR_SUCCESS;
}
cnc::Error UnitRegistry::bind(cnc::EntityId entity, cnc::DefinitionId unit) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid() || unit.value == 0U) return FT_ERR_INVALID_ARGUMENT;
    if (_catalog->find_unit(unit) == nullptr) return FT_ERR_NOT_FOUND;
    if (find(entity) != nullptr) return FT_ERR_ALREADY_EXISTS;
    try { std::vector<Binding> projected = _bindings; projected.push_back({entity, unit});
        std::sort(projected.begin(), projected.end(), [](const Binding &a, const Binding &b) noexcept { return a.entity.value < b.entity.value; });
        _bindings.swap(projected); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}
cnc::Error UnitRegistry::unbind(cnc::EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    auto i = std::find_if(_bindings.begin(), _bindings.end(), [entity](const Binding &b) noexcept { return b.entity.value == entity.value; });
    if (i == _bindings.end()) return FT_ERR_NOT_FOUND;
    _bindings.erase(i); return FT_ERR_SUCCESS;
}
const UnitRegistry::Binding *UnitRegistry::find(cnc::EntityId entity) const noexcept
{
    if (_initialized != FT_TRUE || !entity.is_valid()) return nullptr;
    auto i = std::lower_bound(_bindings.begin(), _bindings.end(), entity.value, [](const Binding &b, uint64_t value) noexcept { return b.entity.value < value; });
    return i != _bindings.end() && i->entity.value == entity.value ? &*i : nullptr;
}
cnc::Error UnitRegistry::export_snapshot(Snapshot *out) const noexcept
{
    if (out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try { out->schema_version = 1U; out->bindings = _bindings; }
    catch (...) { out->bindings.clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}
cnc::Error UnitRegistry::import_snapshot(const Snapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U || snapshot.bindings.size() > (1U << 20U)) return FT_ERR_CONFIGURATION;
    try { std::vector<Binding> restored; restored.reserve(snapshot.bindings.size());
        for (cnc::Size i=0U;i<snapshot.bindings.size();++i) { const Binding &b=snapshot.bindings[i];
            if (!b.entity.is_valid() || _catalog->find_unit(b.unit)==nullptr || (i!=0U && snapshot.bindings[i-1U].entity.value>=b.entity.value)) return FT_ERR_CONFIGURATION;
            restored.push_back(b); }
        _bindings.swap(restored); }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}
void UnitRegistry::swap(UnitRegistry &other) noexcept { _bindings.swap(other._bindings); std::swap(_catalog, other._catalog); std::swap(_initialized, other._initialized); }
uint64_t UnitRegistry::canonical_state_hash() const noexcept
{
    if (_initialized != FT_TRUE) return 0U;
    uint64_t h=1469598103934665603ULL;
    const auto mix=[&h](uint64_t v) noexcept { for(uint32_t s=0U;s<64U;s+=8U){h^=(v>>s)&0xFFU;h*=1099511628211ULL;} };
    for(const Binding &b:_bindings){mix(b.entity.value);mix(b.unit.value);} return h;
}
cnc::Size UnitRegistry::size() const noexcept { return static_cast<cnc::Size>(_bindings.size()); }
}
