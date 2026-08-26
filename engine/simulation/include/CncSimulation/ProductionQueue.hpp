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
    Error initialize() noexcept;
    Error shutdown() noexcept;
    Error enqueue(EntityId producer, DefinitionId definition,
                  SimulationTick now, SimulationTick duration) noexcept;
    Error cancel(EntityId producer, uint64_t sequence) noexcept;
    Error collect_ready(SimulationTick now,
                        std::vector<ProductionOrder> *completed_out) noexcept;
    Error discard() noexcept;
    Size pending_count() const noexcept;

private:
    std::vector<ProductionOrder> _orders;
    uint64_t _next_sequence = 0U;
    Bool _initialized = FT_FALSE;
};

}
