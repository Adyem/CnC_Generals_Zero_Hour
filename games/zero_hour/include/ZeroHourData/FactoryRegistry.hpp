#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation\World.hpp"
#include "ZeroHourData\Catalog.hpp"

namespace zero_hour
{

class FactoryRegistry final
{
public:
    struct Binding
    {
        cnc::EntityId entity;
        cnc::DefinitionId factory;
    };
    struct Snapshot
    {
        uint32_t schema_version = 1U;
        std::vector<Binding> bindings;
    };

    FactoryRegistry() noexcept = default;
    FactoryRegistry(const FactoryRegistry &) = delete;
    FactoryRegistry &operator=(const FactoryRegistry &) = delete;

    cnc::Error initialize(const Catalog *catalog) noexcept;
    cnc::Error shutdown() noexcept;
    cnc::Error bind(cnc::EntityId entity, cnc::DefinitionId factory) noexcept;
    cnc::Error unbind(cnc::EntityId entity) noexcept;
    const Binding *find(cnc::EntityId entity) const noexcept;
    cnc::Error validate_production(cnc::EntityId factory_entity,
                                   cnc::DefinitionId unit,
                                   cnc::Size pending_count) const noexcept;
    cnc::Error export_snapshot(Snapshot *out) const noexcept;
    cnc::Error import_snapshot(const Snapshot &snapshot) noexcept;
    uint64_t canonical_state_hash() const noexcept;
    cnc::Size size() const noexcept;

private:
    const Catalog *_catalog = nullptr;
    std::vector<Binding> _bindings;
    cnc::Bool _initialized = FT_FALSE;
};

}
