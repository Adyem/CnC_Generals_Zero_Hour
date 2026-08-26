#pragma once

#include <cstdint>
#include <vector>

#include "CncRuntime/Types.hpp"
#include "ZeroHourData/Catalog.hpp"

namespace zero_hour
{

class GeneralRoster final
{
public:
    struct SnapshotEntry { cnc::EntityId entity; cnc::DefinitionId general; };
    struct Snapshot { uint32_t schema_version = 1U; std::vector<SnapshotEntry> bindings; };
    cnc::Error initialize(const Catalog *catalog) noexcept;
    cnc::Error assign(cnc::EntityId entity, cnc::DefinitionId general) noexcept;
    cnc::Error find(cnc::EntityId entity, cnc::DefinitionId *general_out) const noexcept;
    cnc::Size size() const noexcept;
    cnc::Error export_snapshot(Snapshot *snapshot_out) const noexcept;
    cnc::Error import_snapshot(const Snapshot &snapshot) noexcept;
    void swap(GeneralRoster &other) noexcept;
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
