#pragma once

#include <cstdint>

#include "CncSimulation/DefinitionRegistry.hpp"

namespace zero_hour
{

// Game-owned records: Libft provides the registry/lifetime contract, while
// Zero Hour owns the meaning and concrete fields of each record.
struct ScienceDefinition
{
    cnc::DefinitionId id;
    uint32_t purchase_cost = 0U;
    uint32_t required_rank = 0U;
};

struct FactionDefinition
{
    cnc::DefinitionId id;
    cnc::DefinitionId starting_science;
};

struct GeneralDefinition
{
    cnc::DefinitionId id;
    cnc::DefinitionId faction;
    cnc::DefinitionId signature_power;
};

struct SpecialPowerDefinition
{
    cnc::DefinitionId id;
    uint32_t recharge_ticks = 0U;
    uint32_t energy_cost = 0U;
};

class Catalog final
{
public:
    static constexpr cnc::DefinitionType science_type{0x5A485343U};
    static constexpr cnc::DefinitionType faction_type{0x5A484641U};
    static constexpr cnc::DefinitionType general_type{0x5A484745U};
    static constexpr cnc::DefinitionType special_power_type{0x5A485350U};

    Catalog() noexcept = default;
    ~Catalog() noexcept;
    Catalog(const Catalog &) = delete;
    Catalog &operator=(const Catalog &) = delete;

    cnc::Error initialize() noexcept;
    cnc::Error install_default_definitions() noexcept;
    cnc::Error shutdown() noexcept;
    cnc::Error validate(cnc::ValidationReport &report) const noexcept;
    const ScienceDefinition *find_science(cnc::DefinitionId id) const noexcept;
    const FactionDefinition *find_faction(cnc::DefinitionId id) const noexcept;
    const GeneralDefinition *find_general(cnc::DefinitionId id) const noexcept;
    const SpecialPowerDefinition *find_special_power(cnc::DefinitionId id) const noexcept;
    cnc::Size definition_count() const noexcept;

private:
    cnc::DefinitionRegistry _registry;
    bool _initialized = false;
};

}
