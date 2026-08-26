#include "ZeroHourData/ScienceLedger.hpp"

#include "errno.hpp"

namespace zero_hour
{

cnc::Error ScienceLedger::initialize(const Catalog *catalog) noexcept
{
    if (_initialized) return FT_ERR_ALREADY_INITIALISED;
    if (catalog == nullptr) return FT_ERR_INVALID_POINTER;
    _catalog = catalog;
    _purchased.clear();
    _initialized = true;
    return FT_ERR_SUCCESS;
}

cnc::Error ScienceLedger::purchase(cnc::DefinitionId faction,
                                   cnc::DefinitionId science,
                                   uint32_t available_points,
                                   uint32_t *remaining_points) noexcept
{
    if (!_initialized) return FT_ERR_INVALID_STATE;
    if (remaining_points == nullptr) return FT_ERR_INVALID_POINTER;
    if (_catalog->find_faction(faction) == nullptr ||
        _catalog->find_science(science) == nullptr)
        return FT_ERR_NOT_FOUND;
    if (is_purchased(science)) return FT_ERR_ALREADY_EXISTS;
    const auto *definition = _catalog->find_science(science);
    if (available_points < definition->purchase_cost) return FT_ERR_OUT_OF_RANGE;
    try
    {
        _purchased.push_back(science);
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    *remaining_points = available_points - definition->purchase_cost;
    return FT_ERR_SUCCESS;
}

bool ScienceLedger::is_purchased(cnc::DefinitionId science) const noexcept
{
    for (const auto purchased : _purchased)
        if (purchased.value == science.value) return true;
    return false;
}

cnc::Size ScienceLedger::purchase_count() const noexcept
{
    return static_cast<cnc::Size>(_purchased.size());
}

cnc::Error ScienceLedger::shutdown() noexcept
{
    _purchased.clear();
    _catalog = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
