#pragma once

#include <cstdint>
#include <vector>

#include "CncSimulation/DefinitionRegistry.hpp"
#include "CncSimulation/World.hpp"

namespace cnc
{

struct ProductionOrder
{
    EntityId producer;
    DefinitionId definition;
    SimulationTick ready_at;
    uint64_t sequence = 0U;
};

class ProductionQueue final
{
public:
    struct Snapshot { uint32_t schema_version = 1U; std::vector<ProductionOrder> orders; uint64_t next_sequence = 0U; };
    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error enqueue(EntityId producer, DefinitionId definition,
                  SimulationTick now, SimulationTick duration) noexcept;
    Error cancel(EntityId producer, uint64_t sequence) noexcept;
    Error collect_ready(SimulationTick now,
                        std::vector<ProductionOrder> *completed_out) noexcept;
    Error discard() noexcept;
    Size pending_count() const noexcept;
    uint64_t canonical_state_hash() const noexcept;
    Error export_snapshot(Snapshot *snapshot_out) const noexcept;
    Error import_snapshot(const Snapshot &snapshot) noexcept;
    void swap(ProductionQueue &other) noexcept;

private:
    std::vector<ProductionOrder> _orders;
    uint64_t _next_sequence = 0U;
    Bool _initialized = FT_FALSE;
};

}
