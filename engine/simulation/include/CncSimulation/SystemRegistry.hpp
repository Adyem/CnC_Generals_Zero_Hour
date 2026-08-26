#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "CncRuntime/Types.hpp"

namespace cnc
{

enum class SystemPhase : uint8_t
{
    ingest_commands = 0U,
    simulation = 1U,
    presentation = 2U
};

using SystemCallback = Error (*)(void *user_data, SystemPhase phase,
                                 SimulationTick tick) noexcept;

class SystemRegistry final
{
public:
    Error add(SystemPhase phase, int32_t order, const char *name,
              SystemCallback callback, void *user_data) noexcept;
    Error run(SystemPhase phase, SimulationTick tick) noexcept;
    Error clear() noexcept;
    Size size() const noexcept;

private:
    struct Entry
    {
        SystemPhase phase;
        int32_t order;
        uint64_t sequence;
        std::string name;
        SystemCallback callback;
        void *user_data;
    };

    std::vector<Entry> _entries;
    uint64_t _next_sequence = 0U;
};

}
