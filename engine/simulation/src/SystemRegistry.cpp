#include "CncSimulation/SystemRegistry.hpp"

#include <algorithm>
#include <limits>

namespace cnc
{

Error SystemRegistry::add(SystemPhase phase, int32_t order, const char *name,
                          SystemCallback callback, void *user_data) noexcept
{
    if (name == nullptr || callback == nullptr)
        return FT_ERR_INVALID_ARGUMENT;
    if (_next_sequence == std::numeric_limits<uint64_t>::max())
        return FT_ERR_OUT_OF_RANGE;
    const uint64_t sequence = _next_sequence;
    std::vector<Entry> projected;
    try
    {
        projected = _entries;
        projected.push_back(Entry{phase, order, sequence, name,
                                  callback, user_data});
        std::stable_sort(projected.begin(), projected.end(),
        [](const Entry &left, const Entry &right)
        {
            if (left.phase != right.phase)
                return static_cast<uint8_t>(left.phase) <
                       static_cast<uint8_t>(right.phase);
            if (left.order != right.order)
                return left.order < right.order;
            return left.sequence < right.sequence;
        });
    }
    catch (...)
    {
        return FT_ERR_NO_MEMORY;
    }
    _entries.swap(projected);
    ++_next_sequence;
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
