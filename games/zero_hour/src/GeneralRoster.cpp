#include "ZeroHourData/GeneralRoster.hpp"

#include "errno.hpp"
#include <algorithm>
#include <utility>

namespace zero_hour
{

cnc::Error GeneralRoster::initialize(const Catalog *catalog) noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr) return FT_ERR_INVALID_POINTER;
    _catalog = catalog;
    _bindings.clear();
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error GeneralRoster::assign(cnc::EntityId entity,
                                  cnc::DefinitionId general) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (!entity.is_valid() || general.value == 0U) return FT_ERR_INVALID_ARGUMENT;
    if (_catalog->find_general(general) == nullptr) return FT_ERR_NOT_FOUND;
    for (const auto &binding : _bindings)
        if (binding.entity.value == entity.value) return FT_ERR_ALREADY_EXISTS;
    try
    {
        _bindings.push_back(Binding{entity, general});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

cnc::Error GeneralRoster::find(cnc::EntityId entity,
                               cnc::DefinitionId *general_out) const noexcept
{
    if (general_out == nullptr) return FT_ERR_INVALID_POINTER;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    for (const auto &binding : _bindings)
    {
        if (binding.entity.value == entity.value)
        {
            *general_out = binding.general;
            return FT_ERR_SUCCESS;
        }
    }
    return FT_ERR_NOT_FOUND;
}

cnc::Size GeneralRoster::size() const noexcept
{
    return static_cast<cnc::Size>(_bindings.size());
}

cnc::Error GeneralRoster::export_snapshot(Snapshot *out) const noexcept
{
    if (out == nullptr) return FT_ERR_INVALID_POINTER;
    if (!_initialized) return FT_ERR_INVALID_STATE;
    try {
        out->schema_version = 1U; out->bindings.clear(); out->bindings.reserve(_bindings.size());
        for (const Binding &b : _bindings) out->bindings.push_back(SnapshotEntry{b.entity, b.general});
        std::sort(out->bindings.begin(), out->bindings.end(), [](const SnapshotEntry &a, const SnapshotEntry &b) noexcept { return a.entity.value < b.entity.value; });
    } catch (...) { out->bindings.clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

cnc::Error GeneralRoster::import_snapshot(const Snapshot &snapshot) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (snapshot.schema_version != 1U || snapshot.bindings.size() > (1U << 20U)) return FT_ERR_CONFIGURATION;
    std::vector<Binding> restored;
    try {
        restored.reserve(snapshot.bindings.size());
        for (cnc::Size i = 0U; i < snapshot.bindings.size(); ++i) {
            const SnapshotEntry &b = snapshot.bindings[i];
            if (!b.entity.is_valid() || b.general.value == 0U || _catalog->find_general(b.general) == nullptr ||
                (i != 0U && snapshot.bindings[i - 1U].entity.value >= b.entity.value)) return FT_ERR_CONFIGURATION;
            restored.push_back(Binding{b.entity, b.general});
        }
    } catch (...) { return FT_ERR_NO_MEMORY; }
    _bindings.swap(restored); return FT_ERR_SUCCESS;
}

void GeneralRoster::swap(GeneralRoster &other) noexcept
{
    _bindings.swap(other._bindings); std::swap(_catalog, other._catalog); std::swap(_initialized, other._initialized);
}

cnc::Error GeneralRoster::shutdown() noexcept
{
    _bindings.clear();
    _catalog = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
