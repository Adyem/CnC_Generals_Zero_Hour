#include "ZeroHourData/Catalog.hpp"

#include <new>
#include <charconv>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

#include "errno.hpp"

namespace zero_hour
{
namespace
{
constexpr cnc::DefinitionId first_science{1U};
constexpr cnc::DefinitionId usa_faction{1U};
constexpr cnc::DefinitionId usa_general{1U};
constexpr cnc::DefinitionId signature_power{1U};

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
void destroy_general(void *raw) noexcept { delete static_cast<GeneralDefinition *>(raw); }
void destroy_special_power(void *raw) noexcept { delete static_cast<SpecialPowerDefinition *>(raw); }
cnc::Error validate_general(const void *raw, cnc::ValidationReport &report) noexcept;
cnc::Error validate_special_power(const void *raw, cnc::ValidationReport &report) noexcept;

cnc::Error register_types(cnc::DefinitionRegistry &registry) noexcept
{
    const cnc::DefinitionTypeDescriptor science{Catalog::science_type, "zero_hour.science", &validate_science, &destroy_science};
    const cnc::DefinitionTypeDescriptor faction{Catalog::faction_type, "zero_hour.faction", &validate_faction, &destroy_faction};
    const cnc::DefinitionTypeDescriptor general{Catalog::general_type, "zero_hour.general", &validate_general, &destroy_general};
    const cnc::DefinitionTypeDescriptor power{Catalog::special_power_type, "zero_hour.special_power", &validate_special_power, &destroy_special_power};
    const cnc::DefinitionTypeDescriptor descriptors[] = {science, faction, general, power};
    for (const auto &descriptor : descriptors)
    {
        const cnc::Error error = registry.register_type(descriptor);
        if (error != FT_ERR_SUCCESS) return error;
    }
    return FT_ERR_SUCCESS;
}

bool parse_u64(const std::string &token, uint64_t *value) noexcept
{
    if (token.empty() || value == nullptr) return false;
    const char *begin = token.data();
    const char *end = begin + token.size();
    const auto result = std::from_chars(begin, end, *value);
    return result.ec == std::errc() && result.ptr == end;
}

cnc::Error validate_general(const void *raw, cnc::ValidationReport &report) noexcept
{
    if (raw == nullptr) return FT_ERR_INVALID_POINTER;
    const auto &value = *static_cast<const GeneralDefinition *>(raw);
    if (value.id.value == 0U || value.faction.value == 0U || value.signature_power.value == 0U)
        ++report.issue_count;
    return FT_ERR_SUCCESS;
}

cnc::Error validate_special_power(const void *raw, cnc::ValidationReport &report) noexcept
{
    if (raw == nullptr) return FT_ERR_INVALID_POINTER;
    const auto &value = *static_cast<const SpecialPowerDefinition *>(raw);
    if (value.id.value == 0U || value.recharge_ticks == 0U) ++report.issue_count;
    return FT_ERR_SUCCESS;
}
}

Catalog::~Catalog() noexcept { (void)shutdown(); }

cnc::Error Catalog::initialize() noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    const cnc::Error error = register_types(_registry);
    if (error != FT_ERR_SUCCESS) { (void)_registry.clear(); return error; }
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error Catalog::load_manifest(const char *path) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (path == nullptr) return FT_ERR_INVALID_POINTER;
    if (_registry.definition_count() != 0U) return FT_ERR_ALREADY_EXISTS;
    std::ifstream input(path);
    if (!input.is_open()) return FT_ERR_FILE_OPEN_FAILED;
    try
    {
    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty() || line[0] == '#') continue;
        std::vector<std::string> fields;
        std::size_t start = 0U;
        while (start <= line.size())
        {
            const std::size_t comma = line.find(',', start);
            fields.push_back(line.substr(start, comma == std::string::npos ? std::string::npos : comma - start));
            if (comma == std::string::npos) break;
            start = comma + 1U;
        }
        uint64_t values[3] = {0U, 0U, 0U};
        if (fields.empty() || fields.size() > 4U) return FT_ERR_INVALID_ARGUMENT;
        const std::size_t value_count = fields.size() - 1U;
        if (value_count > 3U) return FT_ERR_INVALID_ARGUMENT;
        for (std::size_t index = 0U; index < value_count; ++index)
            if (!parse_u64(fields[index + 1U], &values[index])) return FT_ERR_INVALID_ARGUMENT;
        void *record = nullptr;
        cnc::DefinitionType type;
        cnc::DefinitionId id{values[0]};
        if (fields[0] == "SCIENCE" && fields.size() == 4U && values[1] <= std::numeric_limits<uint32_t>::max() && values[2] <= std::numeric_limits<uint32_t>::max())
            record = new (std::nothrow) ScienceDefinition{id, static_cast<uint32_t>(values[1]), static_cast<uint32_t>(values[2])}, type = science_type;
        else if (fields[0] == "FACTION" && fields.size() == 3U)
            record = new (std::nothrow) FactionDefinition{id, cnc::DefinitionId{values[1]}}, type = faction_type;
        else if (fields[0] == "GENERAL" && fields.size() == 4U)
            record = new (std::nothrow) GeneralDefinition{id, cnc::DefinitionId{values[1]}, cnc::DefinitionId{values[2]}}, type = general_type;
        else if (fields[0] == "POWER" && fields.size() == 4U && values[1] <= std::numeric_limits<uint32_t>::max() && values[2] <= std::numeric_limits<uint32_t>::max())
            record = new (std::nothrow) SpecialPowerDefinition{id, static_cast<uint32_t>(values[1]), static_cast<uint32_t>(values[2])}, type = special_power_type;
        else return FT_ERR_INVALID_ARGUMENT;
        if (record == nullptr) return FT_ERR_NO_MEMORY;
        const cnc::Error error = _registry.register_definition(type, id, record);
        if (error != FT_ERR_SUCCESS)
        {
            if (type.value == science_type.value) destroy_science(record);
            else if (type.value == faction_type.value) destroy_faction(record);
            else if (type.value == general_type.value) destroy_general(record);
            else destroy_special_power(record);
            return error;
        }
    }
    cnc::ValidationReport report;
    return validate(report);
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
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
    auto *general = new (std::nothrow) GeneralDefinition{usa_general, usa_faction, signature_power};
    if (general == nullptr) return FT_ERR_NO_MEMORY;
    error = _registry.register_definition(general_type, usa_general, general);
    if (error != FT_ERR_SUCCESS) { delete general; return error; }
    auto *power = new (std::nothrow) SpecialPowerDefinition{signature_power, 60U, 1U};
    if (power == nullptr) return FT_ERR_NO_MEMORY;
    error = _registry.register_definition(special_power_type, signature_power, power);
    if (error != FT_ERR_SUCCESS) { delete power; return error; }
    return FT_ERR_SUCCESS;
}

cnc::Error Catalog::shutdown() noexcept
{
    if (!_initialized) return FT_ERR_SUCCESS;
    const cnc::Error error = _registry.clear();
    _initialized = false;
    return error;
}

cnc::Error Catalog::validate(cnc::ValidationReport &report) const noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    return _registry.validate_all(report);
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

const GeneralDefinition *Catalog::find_general(cnc::DefinitionId id) const noexcept
{
    const void *raw = nullptr;
    if (_registry.find_definition(general_type, id, &raw) != FT_ERR_SUCCESS) return nullptr;
    return static_cast<const GeneralDefinition *>(raw);
}

const SpecialPowerDefinition *Catalog::find_special_power(cnc::DefinitionId id) const noexcept
{
    const void *raw = nullptr;
    if (_registry.find_definition(special_power_type, id, &raw) != FT_ERR_SUCCESS) return nullptr;
    return static_cast<const SpecialPowerDefinition *>(raw);
}

cnc::Size Catalog::definition_count() const noexcept { return _registry.definition_count(); }
}
