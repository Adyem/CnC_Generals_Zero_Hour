#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "CncRuntime/Types.hpp"

namespace cnc
{

struct DefinitionType
{
    uint64_t value = 0U;
};

struct DefinitionId
{
    uint64_t value = 0U;
};

struct ValidationReport
{
    uint32_t issue_count = 0U;
};

using DefinitionValidateCallback = Error (*)(const void *definition,
                                             ValidationReport &report) noexcept;
using DefinitionDestroyCallback = void (*)(void *definition) noexcept;

struct DefinitionTypeDescriptor
{
    DefinitionType type;
    const char *name = nullptr;
    DefinitionValidateCallback validate = nullptr;
    DefinitionDestroyCallback destroy = nullptr;
};

class DefinitionRegistry final
{
public:
    DefinitionRegistry() noexcept = default;
    ~DefinitionRegistry() noexcept;
    DefinitionRegistry(const DefinitionRegistry &) = delete;
    DefinitionRegistry &operator=(const DefinitionRegistry &) = delete;

    Error register_type(const DefinitionTypeDescriptor &descriptor) noexcept;
    Error register_definition(DefinitionType type, DefinitionId id,
                              void *definition) noexcept;
    Error find_definition(DefinitionType type, DefinitionId id,
                          const void **definition_out) const noexcept;
    Error validate_all(ValidationReport &report) const noexcept;
    Error clear() noexcept;
    Size type_count() const noexcept;
    Size definition_count() const noexcept;

private:
    struct TypeEntry
    {
        std::string name;
        DefinitionValidateCallback validate = nullptr;
        DefinitionDestroyCallback destroy = nullptr;
    };

    struct DefinitionEntry
    {
        DefinitionType type;
        void *definition = nullptr;
    };

    std::map<uint64_t, TypeEntry> _types;
    std::map<std::pair<uint64_t, uint64_t>, DefinitionEntry> _definitions;
};

}
