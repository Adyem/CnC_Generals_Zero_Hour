#include <cstdint>
#include <iostream>
#include <vector>

#include "basic.hpp"
#include "errno.hpp"
#include "CncRuntime/Runtime.hpp"
#include "CncSimulation/World.hpp"
#include "CncSimulation/SystemRegistry.hpp"

namespace
{
int32_t record_system(void *user_data, cnc::SystemPhase,
                     cnc::SimulationTick tick) noexcept
{
    auto *const output = static_cast<std::vector<uint64_t> *>(user_data);
    output->push_back(tick.value);
    return FT_ERR_SUCCESS;
}
}

int main()
{
    const char *const text = "Command & Conquer";
    if (ft_strlen_size_t(text) != static_cast<ft_size_t>(17U))
        return 1;

    ft_size_t product = 0U;
    const int32_t multiply_error = ft_size_multiply_checked(
        static_cast<ft_size_t>(6U), static_cast<ft_size_t>(7U), &product);
    if (multiply_error != FT_ERR_SUCCESS || product != static_cast<ft_size_t>(42U))
        return 2;

    cnc::Runtime runtime;
    if (runtime.initialize() != FT_ERR_SUCCESS || runtime.is_initialized() != FT_TRUE)
        return 3;

    ft_size_t checked_sum = 0U;
    if (runtime.checked_add_size(40U, 2U, &checked_sum) != FT_ERR_SUCCESS ||
        checked_sum != 42U)
        return 4;

    const uint64_t before = runtime.monotonic_milliseconds();
    if (before == 0U)
        return 5;
    if (runtime.shutdown() != FT_ERR_SUCCESS || runtime.is_initialized() != FT_FALSE)
        return 6;

    cnc::DeterministicWorld world;
    if (world.initialize() != FT_ERR_SUCCESS)
        return 7;
    cnc::EntityId entity;
    if (world.create_entity(&entity) != FT_ERR_SUCCESS || !entity.is_valid())
        return 8;
    if (world.queue_delta(entity, 40) != FT_ERR_SUCCESS ||
        world.queue_delta(entity, 2) != FT_ERR_SUCCESS ||
        world.advance_one_tick() != FT_ERR_SUCCESS)
        return 9;
    int64_t value = 0;
    if (world.read_value(entity, &value) != FT_ERR_SUCCESS || value != 42 ||
        world.tick().value != 1U || world.canonical_state_hash() == 0U)
        return 10;
    if (world.shutdown() != FT_ERR_SUCCESS)
        return 11;

    cnc::SystemRegistry systems;
    std::vector<uint64_t> execution_ticks;
    if (systems.add(cnc::SystemPhase::simulation, 20, "late", record_system,
                    &execution_ticks) != FT_ERR_SUCCESS ||
        systems.add(cnc::SystemPhase::simulation, 10, "early", record_system,
                    &execution_ticks) != FT_ERR_SUCCESS ||
        systems.size() != static_cast<cnc::Size>(2U) ||
        systems.run(cnc::SystemPhase::simulation, cnc::SimulationTick{7U}) !=
            FT_ERR_SUCCESS || execution_ticks.size() != 2U)
        return 12;

    std::cout << "libft smoke ok (" << CNC_PROJECT_VERSION << ")\n";
    return 0;
}
