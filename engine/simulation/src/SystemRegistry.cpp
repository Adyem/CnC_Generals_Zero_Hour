#include "CncSimulation/SystemRegistry.hpp"

#include <algorithm>

namespace cnc
{

Error SystemRegistry::add(SystemPhase phase, int32_t order, const char *name,
                          SystemCallback callback, void *user_data) noexcept
{
    if (name == nullptr || callback == nullptr)
        return FT_ERR_INVALID_ARGUMENT;
    try
    {
        _entries.push_back(Entry{phase, order, _next_sequence++, name,
                                 callback, user_data});
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    std::stable_sort(_entries.begin(), _entries.end(),
        [](const Entry &left, const Entry &right)
        {
            if (left.phase != right.phase)
                return static_cast<uint8_t>(left.phase) <
                       static_cast<uint8_t>(right.phase);
            if (left.order != right.order)
                return left.order < right.order;
            return left.sequence < right.sequence;
        });
    return FT_ERR_SUCCESS;
}

Error SystemRegistry::run(SystemPhase phase, SimulationTick tick) noexcept
{
    for (Entry &entry : _entries)
    {
        if (entry.phase != phase)
            continue;
        const Error error = entry.callback(entry.user_data, phase, tick);
        if (error != FT_ERR_SUCCESS)
            return error;
    }
    return FT_ERR_SUCCESS;
}

Error SystemRegistry::clear() noexcept
{
    _entries.clear();
    _next_sequence = 0U;
    return FT_ERR_SUCCESS;
}

Size SystemRegistry::size() const noexcept
{
    return static_cast<Size>(_entries.size());
}

}
