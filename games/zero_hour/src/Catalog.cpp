#include "ZeroHourData/Catalog.hpp"

#include <new>

#include "errno.hpp"

namespace zero_hour
{
namespace
{
constexpr cnc::DefinitionId first_science{1U};
constexpr cnc::DefinitionId usa_faction{1U};

cnc::Error validate_science(const void *raw, cnc::ValidationReport &report) noexcept
{
    if (raw == nullptr) return FT_ERR_INVALID_POINTER;
    if (static_cast<const ScienceDefinition *>(raw)->id.value == 0U) ++report.issue_count;
    return FT_ERR_SUCCESS;
}

cnc::Error validate_faction(const void *raw, cnc::ValidationReport &report) noexcept
{
    if (raw == nullptr) return FT_ERR_INVALID_POINTER;
    const auto &faction = *static_cast<const FactionDefinition *>(raw);
    if (faction.id.value == 0U || faction.starting_science.value == 0U) ++report.issue_count;
    return FT_ERR_SUCCESS;
}

void destroy_science(void *raw) noexcept { delete static_cast<ScienceDefinition *>(raw); }
void destroy_faction(void *raw) noexcept { delete static_cast<FactionDefinition *>(raw); }
}

Catalog::~Catalog() noexcept { (void)shutdown(); }

cnc::Error Catalog::initialize() noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    const cnc::DefinitionTypeDescriptor science{science_type, "zero_hour.science", &validate_science, &destroy_science};
    const cnc::DefinitionTypeDescriptor faction{faction_type, "zero_hour.faction", &validate_faction, &destroy_faction};
    cnc::Error error = _registry.register_type(science);
    if (error != FT_ERR_SUCCESS) return error;
    error = _registry.register_type(faction);
    if (error != FT_ERR_SUCCESS) { (void)_registry.clear(); return error; }
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error Catalog::install_default_definitions() noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    auto *science = new (std::nothrow) ScienceDefinition{first_science, 1U, 0U};
    if (science == nullptr) return FT_ERR_NO_MEMORY;
    cnc::Error error = _registry.register_definition(science_type, first_science, science);
    if (error != FT_ERR_SUCCESS) { delete science; return error; }
    auto *faction = new (std::nothrow) FactionDefinition{usa_faction, first_science};
    if (faction == nullptr) return FT_ERR_NO_MEMORY;
    error = _registry.register_definition(faction_type, usa_faction, faction);
    if (error != FT_ERR_SUCCESS) { delete faction; return error; }
    return FT_ERR_SUCCESS;
}

cnc::Error Catalog::shutdown() noexcept
{
    if (!_initialized) return FT_ERR_SUCCESS;
    const cnc::Error error = _registry.clear();
    _initialized = false;
    return error;
}

const ScienceDefinition *Catalog::find_science(cnc::DefinitionId id) const noexcept
{
    const void *raw = nullptr;
    if (_registry.find_definition(science_type, id, &raw) != FT_ERR_SUCCESS) return nullptr;
    return static_cast<const ScienceDefinition *>(raw);
}

const FactionDefinition *Catalog::find_faction(cnc::DefinitionId id) const noexcept
{
    const void *raw = nullptr;
    if (_registry.find_definition(faction_type, id, &raw) != FT_ERR_SUCCESS) return nullptr;
    return static_cast<const FactionDefinition *>(raw);
}

cnc::Size Catalog::definition_count() const noexcept { return _registry.definition_count(); }
}
