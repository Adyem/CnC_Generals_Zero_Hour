#include "ZeroHourData/FactoryRegistry.hpp"

#include <algorithm>
#include <limits>

#include "errno.hpp"

namespace zero_hour
{

cnc::Error FactoryRegistry::initialize(const Catalog *catalog) noexcept
{
    if (_initialized == FT_TRUE) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr) return FT_ERR_INVALID_POINTER;
    _catalog = catalog;
    _bindings.clear();
    _initialized = FT_TRUE;
    return FT_ERR_SUCCESS;
}

cnc::Error FactoryRegistry::shutdown() noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_SUCCESS;
    _bindings.clear();
    _catalog = nullptr;
    _initialized = FT_FALSE;
    return FT_ERR_SUCCESS;
}

cnc::Error FactoryRegistry::bind(cnc::EntityId entity, cnc::DefinitionId factory) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid() || factory.value == 0U) return FT_ERR_INVALID_ARGUMENT;
    if (_catalog->find_factory(factory) == nullptr) return FT_ERR_NOT_FOUND;
    if (find(entity) != nullptr) return FT_ERR_ALREADY_EXISTS;
    try
    {
        std::vector<Binding> projected = _bindings;
        projected.push_back(Binding{entity, factory});
        std::stable_sort(projected.begin(), projected.end(),
                         [](const Binding &left, const Binding &right) noexcept
                         { return left.entity.value < right.entity.value; });
        _bindings.swap(projected);
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

cnc::Error FactoryRegistry::unbind(cnc::EntityId entity) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (!entity.is_valid()) return FT_ERR_INVALID_ARGUMENT;
    auto iterator = std::find_if(_bindings.begin(), _bindings.end(),
                                 [entity](const Binding &binding) noexcept
                                 { return binding.entity.value == entity.value; });
    if (iterator == _bindings.end()) return FT_ERR_NOT_FOUND;
    _bindings.erase(iterator);
    return FT_ERR_SUCCESS;
}

const FactoryRegistry::Binding *FactoryRegistry::find(cnc::EntityId entity) const noexcept
{
    if (_initialized != FT_TRUE || !entity.is_valid()) return nullptr;
    auto iterator = std::lower_bound(_bindings.begin(), _bindings.end(), entity.value,
                                     [](const Binding &binding, uint64_t value) noexcept
                                     { return binding.entity.value < value; });
    return iterator != _bindings.end() && iterator->entity.value == entity.value ? &*iterator : nullptr;
}

cnc::Error FactoryRegistry::validate_production(cnc::EntityId factory_entity,
                                                cnc::DefinitionId unit,
                                                cnc::Size pending_count) const noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    const Binding *binding = find(factory_entity);
    if (binding == nullptr) return FT_ERR_NOT_FOUND;
    const FactoryDefinition *factory_definition = _catalog->find_factory(binding->factory);
    const UnitDefinition *unit_definition = _catalog->find_unit(unit);
    if (factory_definition == nullptr || unit_definition == nullptr) return FT_ERR_NOT_FOUND;
    if (factory_definition->faction.value != unit_definition->faction.value)
        return FT_ERR_INVALID_OPERATION;
    if (pending_count >= static_cast<cnc::Size>(factory_definition->queue_capacity))
        return FT_ERR_OUT_OF_RANGE;
    return FT_ERR_SUCCESS;
}

cnc::Size FactoryRegistry::size() const noexcept
{
    return static_cast<cnc::Size>(_bindings.size());
}

cnc::Error FactoryRegistry::export_snapshot(Snapshot *out) const noexcept
{
    if (out == nullptr) return FT_ERR_INVALID_POINTER;
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    try
    {
        out->schema_version = 1U;
        out->bindings = _bindings;
    }
    catch (...) { out->bindings.clear(); return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

cnc::Error FactoryRegistry::import_snapshot(const Snapshot &snapshot) noexcept
{
    if (_initialized != FT_TRUE) return FT_ERR_NOT_INITIALISED;
    if (snapshot.schema_version != 1U || snapshot.bindings.size() > (1U << 20U))
        return FT_ERR_CONFIGURATION;
    try
    {
        std::vector<Binding> restored;
        restored.reserve(snapshot.bindings.size());
        for (cnc::Size i = 0U; i < snapshot.bindings.size(); ++i)
        {
            const Binding &binding = snapshot.bindings[i];
            if (!binding.entity.is_valid() || _catalog->find_factory(binding.factory) == nullptr ||
                (i != 0U && snapshot.bindings[i - 1U].entity.value >= binding.entity.value))
                return FT_ERR_CONFIGURATION;
            restored.push_back(binding);
        }
        _bindings.swap(restored);
    }
    catch (...) { return FT_ERR_NO_MEMORY; }
    return FT_ERR_SUCCESS;
}

void FactoryRegistry::swap(FactoryRegistry &other) noexcept
{
    _bindings.swap(other._bindings);
    std::swap(_catalog, other._catalog);
    std::swap(_initialized, other._initialized);
}

uint64_t FactoryRegistry::canonical_state_hash() const noexcept
{
    if (_initialized != FT_TRUE) return 0U;
    uint64_t hash = 1469598103934665603ULL;
    const auto mix = [&hash](uint64_t value) noexcept
    {
        for (uint32_t shift = 0U; shift < 64U; shift += 8U)
        {
            hash ^= (value >> shift) & 0xFFU;
            hash *= 1099511628211ULL;
        }
    };
    for (const Binding &binding : _bindings)
    {
        mix(binding.entity.value);
        mix(binding.factory.value);
    }
    return hash;
}

}
