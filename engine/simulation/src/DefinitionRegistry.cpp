#include "CncSimulation/DefinitionRegistry.hpp"

namespace cnc
{

DefinitionRegistry::~DefinitionRegistry() noexcept
{
    clear();
}

Error DefinitionRegistry::register_type(const DefinitionTypeDescriptor &descriptor) noexcept
{
    if (descriptor.type.value == 0U || descriptor.name == nullptr ||
        descriptor.destroy == nullptr)
        return FT_ERR_INVALID_ARGUMENT;
    if (_types.find(descriptor.type.value) != _types.end())
        return FT_ERR_ALREADY_EXISTS;
    try
    {
        _types.emplace(descriptor.type.value,
                       TypeEntry{descriptor.name, descriptor.validate,
                                 descriptor.destroy});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error DefinitionRegistry::register_definition(DefinitionType type, DefinitionId id,
                                               void *definition) noexcept
{
    if (type.value == 0U || id.value == 0U || definition == nullptr)
        return FT_ERR_INVALID_ARGUMENT;
    const auto type_iterator = _types.find(type.value);
    if (type_iterator == _types.end())
        return FT_ERR_NOT_FOUND;
    const std::pair<uint64_t, uint64_t> key(type.value, id.value);
    if (_definitions.find(key) != _definitions.end())
        return FT_ERR_ALREADY_EXISTS;
    try
    {
        _definitions.emplace(key, DefinitionEntry{type, definition});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    return FT_ERR_SUCCESS;
}

Error DefinitionRegistry::find_definition(DefinitionType type, DefinitionId id,
                                           const void **definition_out) const noexcept
{
    if (definition_out == nullptr)
        return FT_ERR_INVALID_POINTER;
    const auto iterator = _definitions.find({type.value, id.value});
    if (iterator == _definitions.end())
        return FT_ERR_NOT_FOUND;
    *definition_out = iterator->second.definition;
    return FT_ERR_SUCCESS;
}

Error DefinitionRegistry::validate_all(ValidationReport &report) const noexcept
{
    report.issue_count = 0U;
    for (const auto &definition_pair : _definitions)
    {
        const auto type_iterator = _types.find(definition_pair.second.type.value);
        if (type_iterator == _types.end())
            return FT_ERR_INTERNAL;
        if (type_iterator->second.validate == nullptr)
            continue;
        const Error error = type_iterator->second.validate(
            definition_pair.second.definition, report);
        if (error != FT_ERR_SUCCESS)
            return error;
    }
    return report.issue_count == 0U ? FT_ERR_SUCCESS : FT_ERR_CONFIGURATION;
}

Error DefinitionRegistry::clear() noexcept
{
    for (const auto &definition_pair : _definitions)
    {
        const auto type_iterator = _types.find(definition_pair.second.type.value);
        if (type_iterator != _types.end() && type_iterator->second.destroy != nullptr)
            type_iterator->second.destroy(definition_pair.second.definition);
    }
    _definitions.clear();
    _types.clear();
    return FT_ERR_SUCCESS;
}

Size DefinitionRegistry::type_count() const noexcept
{
    return static_cast<Size>(_types.size());
}

Size DefinitionRegistry::definition_count() const noexcept
{
    return static_cast<Size>(_definitions.size());
}

}
