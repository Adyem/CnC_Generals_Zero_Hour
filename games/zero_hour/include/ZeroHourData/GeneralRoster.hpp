#pragma once

#include <vector>

#include "CncRuntime/Types.hpp"
#include "ZeroHourData/Catalog.hpp"

namespace zero_hour
{

class GeneralRoster final
{
public:
    cnc::Error initialize(const Catalog *catalog) noexcept;
    cnc::Error assign(cnc::EntityId entity, cnc::DefinitionId general) noexcept;
    cnc::Error find(cnc::EntityId entity, cnc::DefinitionId *general_out) const noexcept;
    cnc::Size size() const noexcept;
    cnc::Error shutdown() noexcept;

private:
    struct Binding
    {
        cnc::EntityId entity;
        cnc::DefinitionId general;
    };
    const Catalog *_catalog = nullptr;
    std::vector<Binding> _bindings;
    bool _initialized = false;
};

}
