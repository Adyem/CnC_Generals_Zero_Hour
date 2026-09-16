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
    cnc::Size size() const noexcept;

private:
    const Catalog *_catalog = nullptr;
    std::vector<Binding> _bindings;
    cnc::Bool _initialized = FT_FALSE;
};

}
