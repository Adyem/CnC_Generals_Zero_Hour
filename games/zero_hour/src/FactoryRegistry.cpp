#include "ZeroHourData/FactoryRegistry.hpp"

#include <algorithm>

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

}
