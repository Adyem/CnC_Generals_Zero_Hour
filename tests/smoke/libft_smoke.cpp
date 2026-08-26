#include <cstdint>
#include <iostream>
#include <vector>

#include "basic.hpp"
#include "errno.hpp"
#include "CncRuntime/Runtime.hpp"
#include "CncSimulation/World.hpp"
#include "CncSimulation/SystemRegistry.hpp"
#include "CncSimulation/DefinitionRegistry.hpp"
#include "ZeroHourData/Catalog.hpp"
#include "CncGame/GameSession.hpp"
#include "CncRender/Renderer.hpp"

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

    cnc::DefinitionRegistry definitions;
    if (definitions.register_type(cnc::DefinitionTypeDescriptor{
            cnc::DefinitionType{1U}, "test_definition", nullptr,
            [](void *pointer) noexcept { delete static_cast<uint64_t *>(pointer); }}) !=
            FT_ERR_SUCCESS)
        return 13;
    auto *const definition = new uint64_t(123U);
    if (definitions.register_definition(cnc::DefinitionType{1U},
                                        cnc::DefinitionId{7U}, definition) !=
            FT_ERR_SUCCESS)
        return 14;
    const void *found = nullptr;
    if (definitions.find_definition(cnc::DefinitionType{1U},
                                    cnc::DefinitionId{7U}, &found) != FT_ERR_SUCCESS ||
        found != definition || definitions.definition_count() != static_cast<cnc::Size>(1U) ||
        definitions.clear() != FT_ERR_SUCCESS)
        return 15;

    zero_hour::Catalog catalog;
    if (catalog.initialize() != FT_ERR_SUCCESS ||
        catalog.install_default_definitions() != FT_ERR_SUCCESS ||
        catalog.definition_count() != static_cast<cnc::Size>(4U))
        return 16;
    const auto *science = catalog.find_science(cnc::DefinitionId{1U});
    const auto *faction = catalog.find_faction(cnc::DefinitionId{1U});
    const auto *general = catalog.find_general(cnc::DefinitionId{1U});
    const auto *power = catalog.find_special_power(cnc::DefinitionId{1U});
    cnc::ValidationReport catalog_report;
    if (catalog.validate(catalog_report) != FT_ERR_SUCCESS ||
        catalog_report.issue_count != 0U)
        return 17;
    if (science == nullptr || science->purchase_cost != 1U || faction == nullptr ||
        faction->starting_science.value != science->id.value ||
        general == nullptr || general->faction.value != faction->id.value ||
        power == nullptr || power->recharge_ticks != 60U ||
        catalog.shutdown() != FT_ERR_SUCCESS)
        return 17;

    zero_hour::Catalog manifest_catalog;
#ifdef CNC_ZERO_HOUR_MANIFEST_PATH
    const char *const manifest_path = CNC_ZERO_HOUR_MANIFEST_PATH;
#else
    const char *const manifest_path = "tests/fixtures/zero_hour_manifest.csv";
#endif
    if (manifest_catalog.initialize() != FT_ERR_SUCCESS ||
        manifest_catalog.load_manifest(manifest_path) != FT_ERR_SUCCESS ||
        manifest_catalog.definition_count() != static_cast<cnc::Size>(4U))
        return 18;
    if (manifest_catalog.shutdown() != FT_ERR_SUCCESS)
        return 19;

    zero_hour::Catalog invalid_catalog;
#ifdef CNC_ZERO_HOUR_INVALID_MANIFEST_PATH
    const char *const invalid_manifest_path = CNC_ZERO_HOUR_INVALID_MANIFEST_PATH;
#else
    const char *const invalid_manifest_path = "tests/fixtures/zero_hour_invalid_manifest.csv";
#endif
    cnc::ValidationReport invalid_report;
    if (invalid_catalog.initialize() != FT_ERR_SUCCESS ||
        invalid_catalog.load_manifest(invalid_manifest_path) != FT_ERR_CONFIGURATION ||
        invalid_catalog.validate(invalid_report) != FT_ERR_CONFIGURATION ||
        invalid_report.issue_count == 0U || invalid_catalog.shutdown() != FT_ERR_SUCCESS)
        return 20;

    cnc::GameSession session;
    if (session.initialize() != FT_ERR_SUCCESS ||
        session.install_default_data() != FT_ERR_SUCCESS ||
        session.catalog().definition_count() != static_cast<cnc::Size>(4U))
        return 21;
    cnc::EntityId session_entity;
    if (session.world().create_entity(&session_entity) != FT_ERR_SUCCESS ||
        session.submit_world_delta(session_entity, 5) != FT_ERR_SUCCESS ||
        session.advance_one_tick() != FT_ERR_SUCCESS)
        return 22;
    int64_t session_value = 0;
    if (session.world().read_value(session_entity, &session_value) != FT_ERR_SUCCESS ||
        session_value != 5 || session.world().tick().value != 1U ||
        session.shutdown() != FT_ERR_SUCCESS || session.is_initialized() == FT_TRUE)
        return 23;

    cnc::HeadlessRenderer renderer;
    if (renderer.initialize() != FT_ERR_SUCCESS ||
        renderer.begin_frame() != FT_ERR_SUCCESS ||
        renderer.submit(cnc::RenderCommand{1U, 0, 0, 64U, 64U}) != FT_ERR_SUCCESS ||
        renderer.submitted_command_count() != static_cast<cnc::Size>(1U) ||
        renderer.end_frame() != FT_ERR_SUCCESS || renderer.frame_count() != 1U ||
        renderer.shutdown() != FT_ERR_SUCCESS)
        return 23;

    std::cout << "libft smoke ok (" << CNC_PROJECT_VERSION << ")\n";
    return 0;
}
