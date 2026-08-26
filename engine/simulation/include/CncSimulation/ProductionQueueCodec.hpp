#pragma once
#include <cstdint>
#include <vector>
#include "CncSimulation/ProductionQueue.hpp"
namespace cnc { class ProductionQueueCodec final { public: static constexpr uint32_t wire_schema_version=1U; static constexpr Size max_entries=static_cast<Size>(1U<<20U); static Error encode(const ProductionQueue::Snapshot&,std::vector<uint8_t>*) noexcept; static Error decode(const uint8_t*,Size,ProductionQueue::Snapshot*) noexcept; }; }
