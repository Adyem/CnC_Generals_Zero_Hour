#include "ZeroHourData/ScienceLedger.hpp"

#include "errno.hpp"
#include <algorithm>
#include <utility>

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
cnc::Error ScienceLedger::export_snapshot(Snapshot*out) const noexcept { if(!out)return FT_ERR_INVALID_POINTER; if(!_initialized)return FT_ERR_INVALID_STATE; try{out->schema_version=1U;out->purchased=_purchased;std::sort(out->purchased.begin(),out->purchased.end(),[](auto a,auto b)noexcept{return a.value<b.value;});}catch(...){out->purchased.clear();return FT_ERR_NO_MEMORY;}return FT_ERR_SUCCESS; }
cnc::Error ScienceLedger::import_snapshot(const Snapshot&s) noexcept { if(!_initialized)return FT_ERR_INVALID_STATE;if(s.schema_version!=1U||s.purchased.size()>(1U<<20U))return FT_ERR_CONFIGURATION;std::vector<cnc::DefinitionId> r;try{r.reserve(s.purchased.size());for(cnc::Size i=0;i<s.purchased.size();++i){if(s.purchased[i].value==0U||_catalog->find_science(s.purchased[i])==nullptr||(i&&s.purchased[i-1U].value>=s.purchased[i].value))return FT_ERR_CONFIGURATION;r.push_back(s.purchased[i]);}}catch(...){return FT_ERR_NO_MEMORY;}_purchased.swap(r);return FT_ERR_SUCCESS; }
void ScienceLedger::swap(ScienceLedger&o) noexcept{_purchased.swap(o._purchased);std::swap(_catalog,o._catalog);std::swap(_initialized,o._initialized);}

cnc::Error ScienceLedger::shutdown() noexcept
{
    _purchased.clear();
    _catalog = nullptr;
    _initialized = false;
    return FT_ERR_SUCCESS;
}

}
