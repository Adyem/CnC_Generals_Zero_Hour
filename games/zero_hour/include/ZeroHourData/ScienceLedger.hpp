#pragma once

#include <vector>

#include "ZeroHourData/Catalog.hpp"

namespace zero_hour
{

class ScienceLedger final
{
public:
    ScienceLedger() noexcept = default;
    cnc::Error initialize(const Catalog *catalog) noexcept;
    cnc::Error purchase(cnc::DefinitionId faction, cnc::DefinitionId science,
                        uint32_t available_points, uint32_t *remaining_points) noexcept;
    bool is_purchased(cnc::DefinitionId science) const noexcept;
    cnc::Size purchase_count() const noexcept;
    cnc::Error shutdown() noexcept;

private:
    const Catalog *_catalog = nullptr;
    std::vector<cnc::DefinitionId> _purchased;
    bool _initialized = false;
};

}
