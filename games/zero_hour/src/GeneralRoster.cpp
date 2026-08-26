#include "ZeroHourData/GeneralRoster.hpp"

#include "errno.hpp"

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

cnc::Error GeneralRoster::shutdown() noexcept
{
    _bindings.clear();
    _catalog = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
