#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "basic.hpp"
#include "errno.hpp"
#include "CncRuntime/Runtime.hpp"
#include "CncSimulation/World.hpp"
#include "CncSimulation/SnapshotCodec.hpp"
#include "CncSimulation/CommandCodec.hpp"
#include "CncSimulation/PlayerRegistry.hpp"
#include "CncSimulation/SystemRegistry.hpp"
#include "CncSimulation/DefinitionRegistry.hpp"
#include "ZeroHourData/Catalog.hpp"
#include "ZeroHourData/PlayerState.hpp"
#include "ZeroHourData/ScienceLedger.hpp"
#include "CncGame/GameSession.hpp"
#include "CncRender/Renderer.hpp"
#include "CncNetwork/NetworkSession.hpp"

namespace
{
int32_t record_system(void *user_data, cnc::SystemPhase,
                     cnc::SimulationTick tick) noexcept
{
    auto *const output = static_cast<std::vector<uint64_t> *>(user_data);
    output->push_back(tick.value);
    return FT_ERR_SUCCESS;
}

cnc::Error manifest_reader(const char *, std::string &contents, void *context) noexcept
{
    if (context == nullptr) return FT_ERR_INVALID_POINTER;
    contents = *static_cast<const std::string *>(context);
    return FT_ERR_SUCCESS;
}

uint64_t fixed_clock() noexcept { return 123456U; }

cnc::Error fail_once(void *user_data, cnc::SystemPhase,
                     cnc::SimulationTick) noexcept
{
    auto *failed = static_cast<bool *>(user_data);
    if (!*failed)
    {
        *failed = true;
        return FT_ERR_CONFIGURATION;
    }
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
    if (runtime.set_monotonic_clock(&fixed_clock) != FT_ERR_SUCCESS ||
        runtime.set_monotonic_clock(nullptr) != FT_ERR_INVALID_POINTER)
        return 3;
    if (runtime.initialize() != FT_ERR_SUCCESS || runtime.is_initialized() != FT_TRUE)
        return 3;

    ft_size_t checked_sum = 0U;
    if (runtime.checked_add_size(40U, 2U, &checked_sum) != FT_ERR_SUCCESS ||
        checked_sum != 42U)
        return 4;

    const uint64_t before = runtime.monotonic_milliseconds();
    if (before != 123456U)
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
    cnc::WorldSnapshot snapshot;
    if (world.export_snapshot(&snapshot) != FT_ERR_SUCCESS ||
        snapshot.schema_version != 1U || snapshot.tick.value != 1U ||
        snapshot.entries.size() != 1U || snapshot.entries[0].entity.value != entity.value ||
        snapshot.entries[0].value != 42 || snapshot.entries[0].alive != FT_TRUE)
        return 33;
    std::vector<uint8_t> snapshot_bytes;
    cnc::WorldSnapshot decoded_snapshot;
    if (cnc::WorldSnapshotCodec::encode(snapshot, &snapshot_bytes) != FT_ERR_SUCCESS ||
        cnc::WorldSnapshotCodec::decode(
            snapshot_bytes.data(), static_cast<ft_size_t>(snapshot_bytes.size()),
            &decoded_snapshot) != FT_ERR_SUCCESS ||
        decoded_snapshot.tick.value != snapshot.tick.value ||
        decoded_snapshot.entries.size() != snapshot.entries.size() ||
        decoded_snapshot.entries[0].entity.value != snapshot.entries[0].entity.value ||
        decoded_snapshot.entries[0].value != snapshot.entries[0].value)
        return 37;
    snapshot_bytes[0] = 0U;
    if (cnc::WorldSnapshotCodec::decode(
            snapshot_bytes.data(), static_cast<ft_size_t>(snapshot_bytes.size()),
            &decoded_snapshot) != FT_ERR_CONFIGURATION)
        return 38;
    cnc::WorldSnapshot invalid_snapshot = snapshot;
    invalid_snapshot.entries.push_back(snapshot.entries[0]);
    if (world.import_snapshot(invalid_snapshot) != FT_ERR_CONFIGURATION ||
        world.read_value(entity, &value) != FT_ERR_SUCCESS || value != 42)
        return 40;
    cnc::WorldCommandFrame command_frame;
    command_frame.tick = cnc::SimulationTick{9U};
    command_frame.commands.push_back(cnc::WorldCommand{entity, -7, 1U});
    std::vector<uint8_t> command_bytes;
    cnc::WorldCommandFrame decoded_commands;
    if (cnc::WorldCommandCodec::encode(command_frame, &command_bytes) != FT_ERR_SUCCESS ||
        cnc::WorldCommandCodec::decode(
            command_bytes.data(), static_cast<ft_size_t>(command_bytes.size()),
            &decoded_commands) != FT_ERR_SUCCESS || decoded_commands.tick.value != 9U ||
        decoded_commands.commands.size() != 1U ||
        decoded_commands.commands[0].entity.value != entity.value ||
        decoded_commands.commands[0].delta != -7 ||
        decoded_commands.commands[0].sequence != 1U)
        return 41;
    if (world.queue_delta(entity, INT64_MAX) != FT_ERR_SUCCESS ||
        world.advance_one_tick() != FT_ERR_OUT_OF_RANGE ||
        world.read_value(entity, &value) != FT_ERR_SUCCESS || value != 42)
        return 34;
    if (world.discard_pending_commands() != FT_ERR_SUCCESS)
        return 35;
    if (world.shutdown() != FT_ERR_SUCCESS)
        return 11;

    cnc::PlayerRegistry players;
    cnc::Diplomacy diplomacy = cnc::Diplomacy::hostile;
    cnc::PlayerId owner_id;
    if (players.initialize() != FT_ERR_SUCCESS ||
        players.create_player(cnc::PlayerId{1U}) != FT_ERR_SUCCESS ||
        players.create_player(cnc::PlayerId{2U}) != FT_ERR_SUCCESS ||
        players.set_relationship(cnc::PlayerId{1U}, cnc::PlayerId{2U},
                                 cnc::Diplomacy::hostile) != FT_ERR_SUCCESS ||
        players.set_relationship(cnc::PlayerId{1U}, cnc::PlayerId{2U},
                                 cnc::Diplomacy::allied) != FT_ERR_SUCCESS ||
        players.relationship(cnc::PlayerId{2U}, cnc::PlayerId{1U}, &diplomacy) != FT_ERR_SUCCESS ||
        diplomacy != cnc::Diplomacy::allied || players.player_count() != static_cast<cnc::Size>(2U) ||
        players.set_owner(cnc::EntityId{42U}, cnc::PlayerId{1U}) != FT_ERR_SUCCESS ||
        players.owner(cnc::EntityId{42U}, nullptr) != FT_ERR_INVALID_POINTER ||
        players.owner(cnc::EntityId{42U}, &owner_id) != FT_ERR_SUCCESS || owner_id.value != 1U ||
        players.set_owner(cnc::EntityId{42U}, cnc::PlayerId{2U}) != FT_ERR_SUCCESS ||
        players.remove_player(cnc::PlayerId{1U}) != FT_ERR_SUCCESS ||
        players.relationship(cnc::PlayerId{2U}, cnc::PlayerId{1U}, &diplomacy) != FT_ERR_NOT_FOUND ||
        players.owner(cnc::EntityId{42U}, &owner_id) != FT_ERR_NOT_FOUND ||
        players.shutdown() != FT_ERR_SUCCESS)
        return 44;

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

    zero_hour::Catalog rules_catalog;
    zero_hour::ScienceLedger ledger;
    uint32_t remaining_points = 0U;
    if (rules_catalog.initialize() != FT_ERR_SUCCESS ||
        rules_catalog.install_default_definitions() != FT_ERR_SUCCESS ||
        ledger.initialize(&rules_catalog) != FT_ERR_SUCCESS ||
        ledger.purchase(cnc::DefinitionId{1U}, cnc::DefinitionId{1U}, 5U,
                        &remaining_points) != FT_ERR_SUCCESS ||
        remaining_points != 4U || !ledger.is_purchased(cnc::DefinitionId{1U}) ||
        ledger.purchase_count() != static_cast<cnc::Size>(1U) ||
        ledger.purchase(cnc::DefinitionId{1U}, cnc::DefinitionId{1U}, 5U,
                        &remaining_points) != FT_ERR_ALREADY_EXISTS ||
        ledger.shutdown() != FT_ERR_SUCCESS || rules_catalog.shutdown() != FT_ERR_SUCCESS)
        return 18;

    zero_hour::Catalog manifest_catalog;
#ifdef CNC_ZERO_HOUR_MANIFEST_PATH
    const char *const manifest_path = CNC_ZERO_HOUR_MANIFEST_PATH;
#else
    const char *const manifest_path = "tests/fixtures/zero_hour_manifest.csv";
#endif
    if (manifest_catalog.initialize() != FT_ERR_SUCCESS ||
        manifest_catalog.load_manifest(manifest_path) != FT_ERR_SUCCESS ||
        manifest_catalog.definition_count() != static_cast<cnc::Size>(4U))
        return 19;
    if (manifest_catalog.shutdown() != FT_ERR_SUCCESS)
        return 20;

    zero_hour::Catalog text_catalog;
    if (text_catalog.initialize() != FT_ERR_SUCCESS ||
        text_catalog.load_manifest_text(
            "SCIENCE,1,1,0\nFACTION,1,1\nGENERAL,1,1,1\nPOWER,1,60,1\n") !=
            FT_ERR_SUCCESS ||
        text_catalog.definition_count() != static_cast<cnc::Size>(4U) ||
        text_catalog.shutdown() != FT_ERR_SUCCESS)
        return 21;

    std::string callback_manifest =
        "SCIENCE,1,1,0\nFACTION,1,1\nGENERAL,1,1,1\nPOWER,1,60,1\n";
    zero_hour::Catalog callback_catalog;
    if (callback_catalog.initialize() != FT_ERR_SUCCESS ||
        callback_catalog.load_manifest_with_reader("memory", &manifest_reader,
                                                   &callback_manifest) != FT_ERR_SUCCESS ||
        callback_catalog.definition_count() != static_cast<cnc::Size>(4U) ||
        callback_catalog.shutdown() != FT_ERR_SUCCESS)
        return 22;

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
        return 21;

    cnc::GameSession session;
    if (session.initialize() != FT_ERR_SUCCESS ||
        session.install_default_data() != FT_ERR_SUCCESS ||
        session.catalog().definition_count() != static_cast<cnc::Size>(4U) ||
        session.science_ledger().purchase_count() != static_cast<cnc::Size>(0U) ||
        session.players().create_player(cnc::PlayerId{1U}) != FT_ERR_SUCCESS ||
        session.players().player_count() != static_cast<cnc::Size>(1U))
        return 22;
    if (session.has_game_data() != FT_TRUE)
        return 29;
    if (session.phase() != cnc::GameSession::Phase::data_ready)
        return 33;
    if (session.validate_game_data() != FT_ERR_SUCCESS)
        return 31;
    cnc::GameSession manifest_session;
#ifdef CNC_ZERO_HOUR_MANIFEST_PATH
    const char *const session_manifest_path = CNC_ZERO_HOUR_MANIFEST_PATH;
#else
    const char *const session_manifest_path = "tests/fixtures/zero_hour_manifest.csv";
#endif
    if (manifest_session.initialize() != FT_ERR_SUCCESS ||
        manifest_session.load_data_manifest(session_manifest_path) != FT_ERR_SUCCESS ||
        manifest_session.catalog().definition_count() != static_cast<cnc::Size>(4U))
        return 25;
    if (manifest_session.validate_game_data() != FT_ERR_SUCCESS)
        return 32;
    zero_hour::PlayerState &player = manifest_session.player_state();
    cnc::EntityId player_entity;
    cnc::EntityId replacement_entity;
    cnc::SimulationTick player_power_ready;
    if (player.set_faction(cnc::DefinitionId{1U}) != FT_ERR_SUCCESS ||
        player.set_science_points(2U) != FT_ERR_SUCCESS ||
        player.purchase_science(cnc::DefinitionId{1U}) != FT_ERR_SUCCESS ||
        player.science_points() != 1U ||
        manifest_session.world().create_entity(&player_entity) != FT_ERR_SUCCESS ||
        player.assign_general(player_entity, cnc::DefinitionId{1U}) != FT_ERR_SUCCESS ||
        manifest_session.world().create_entity(&replacement_entity) != FT_ERR_SUCCESS ||
        player.assign_general(replacement_entity, cnc::DefinitionId{1U}) != FT_ERR_INVALID_OPERATION ||
        player.activate_power(cnc::DefinitionId{1U}, cnc::SimulationTick{0U},
                              &player_power_ready) != FT_ERR_SUCCESS ||
        player_power_ready.value != 60U ||
        manifest_session.shutdown() != FT_ERR_SUCCESS)
        return 30;
    uint32_t session_points = 0U;
    if (session.science_ledger().purchase(cnc::DefinitionId{1U}, cnc::DefinitionId{1U},
                                          2U, &session_points) != FT_ERR_SUCCESS ||
        session_points != 1U)
        return 23;
    cnc::SimulationTick power_ready;
    if (session.special_power_ledger().activate(cnc::DefinitionId{1U},
                                                cnc::SimulationTick{0U},
                                                &power_ready) != FT_ERR_SUCCESS ||
        power_ready.value != 60U ||
        session.special_power_ledger().is_ready(cnc::DefinitionId{1U},
                                                cnc::SimulationTick{1U}) ||
        session.special_power_ledger().activate(cnc::DefinitionId{1U},
                                                cnc::SimulationTick{1U},
                                                &power_ready) != FT_ERR_INVALID_OPERATION)
        return 24;
    cnc::EntityId general_entity;
    cnc::DefinitionId assigned_general;
    if (session.world().create_entity(&general_entity) != FT_ERR_SUCCESS ||
        session.general_roster().assign(general_entity, cnc::DefinitionId{1U}) != FT_ERR_SUCCESS ||
        session.general_roster().find(general_entity, &assigned_general) != FT_ERR_SUCCESS ||
        assigned_general.value != 1U || session.general_roster().size() != static_cast<cnc::Size>(1U))
        return 25;
    cnc::EntityId session_entity;
    if (session.world().create_entity(&session_entity) != FT_ERR_SUCCESS ||
        session.submit_world_delta(session_entity, 5) != FT_ERR_SUCCESS ||
        session.advance_one_tick() != FT_ERR_SUCCESS)
        return 26;
    int64_t session_value = 0;
    if (session.world().read_value(session_entity, &session_value) != FT_ERR_SUCCESS ||
        session_value != 5 || session.world().tick().value != 1U ||
        session.replay_history().size() != 1U ||
        session.replay_history()[0].state_hash != session.world().canonical_state_hash() ||
        session.verify_replay(session.replay_history()) != FT_ERR_SUCCESS ||
        session.phase() != cnc::GameSession::Phase::running)
        return 27;

    cnc::GameSession::ReplayRecord divergent_record = session.replay_history().empty()
        ? cnc::GameSession::ReplayRecord{}
        : session.replay_history()[0];
    divergent_record.state_hash ^= static_cast<uint64_t>(1U);
    std::vector<cnc::GameSession::ReplayRecord> divergent_replay{divergent_record};
    if (session.verify_replay(divergent_replay) != FT_ERR_CONFIGURATION)
        return 28;
    std::vector<uint8_t> saved_session;
    int64_t restored_value = 0;
    if (session.save_snapshot(&saved_session) != FT_ERR_SUCCESS ||
        session.submit_world_delta(session_entity, 10) != FT_ERR_SUCCESS ||
        session.advance_one_tick() != FT_ERR_SUCCESS ||
        session.load_snapshot(saved_session.data(),
                              static_cast<cnc::Size>(saved_session.size())) != FT_ERR_SUCCESS ||
        session.world().read_value(session_entity, &restored_value) != FT_ERR_SUCCESS ||
        restored_value != 5 || session.world().tick().value != 1U ||
        !session.replay_history().empty())
        return 39;
    cnc::WorldCommandFrame session_frame;
    session_frame.tick = session.world().tick();
    session_frame.commands.push_back(cnc::WorldCommand{session_entity, 3, 0U});
    std::vector<uint8_t> session_command_bytes;
    if (cnc::WorldCommandCodec::encode(session_frame, &session_command_bytes) != FT_ERR_SUCCESS ||
        session.submit_command_frame(
            session_command_bytes.data(), static_cast<cnc::Size>(session_command_bytes.size())) !=
            FT_ERR_SUCCESS || session.advance_one_tick() != FT_ERR_SUCCESS ||
        session.world().read_value(session_entity, &restored_value) != FT_ERR_SUCCESS ||
        restored_value != 8)
        return 42;
    cnc::WorldCommandFrame mixed_frame;
    mixed_frame.tick = session.world().tick();
    mixed_frame.commands.push_back(cnc::WorldCommand{session_entity, 2, 0U});
    mixed_frame.commands.push_back(cnc::WorldCommand{cnc::EntityId{999U}, 4, 1U});
    std::vector<uint8_t> mixed_bytes;
    if (cnc::WorldCommandCodec::encode(mixed_frame, &mixed_bytes) != FT_ERR_SUCCESS ||
        session.submit_command_frame(
            mixed_bytes.data(), static_cast<cnc::Size>(mixed_bytes.size())) != FT_ERR_NOT_FOUND ||
        session.advance_one_tick() != FT_ERR_SUCCESS ||
        session.world().read_value(session_entity, &restored_value) != FT_ERR_SUCCESS ||
        restored_value != 8)
        return 43;
    if (session.shutdown() != FT_ERR_SUCCESS || session.is_initialized() == FT_TRUE)
        return 29;

    cnc::GameSession retry_session;
    cnc::EntityId retry_entity;
    bool failed_once = false;
    int64_t retry_value = 0;
    if (retry_session.initialize() != FT_ERR_SUCCESS ||
        retry_session.install_default_data() != FT_ERR_SUCCESS ||
        retry_session.world().create_entity(&retry_entity) != FT_ERR_SUCCESS ||
        retry_session.systems().add(cnc::SystemPhase::ingest_commands, 0, "fail_once",
                                    &fail_once, &failed_once) != FT_ERR_SUCCESS ||
        retry_session.submit_world_delta(retry_entity, 7) != FT_ERR_SUCCESS ||
        retry_session.advance_one_tick() != FT_ERR_CONFIGURATION ||
        retry_session.advance_one_tick() != FT_ERR_SUCCESS ||
        retry_session.world().read_value(retry_entity, &retry_value) != FT_ERR_SUCCESS ||
        retry_value != 7 || retry_session.replay_history().size() != 1U ||
        retry_session.shutdown() != FT_ERR_SUCCESS)
        return 36;

    cnc::HeadlessRenderer renderer;
    if (renderer.initialize() != FT_ERR_SUCCESS ||
        renderer.begin_frame() != FT_ERR_SUCCESS ||
        renderer.present_snapshot(snapshot) != FT_ERR_SUCCESS ||
        renderer.submit(cnc::RenderCommand{1U, 0, 0, 64U, 64U}) != FT_ERR_SUCCESS ||
        renderer.submitted_command_count() != static_cast<cnc::Size>(1U) ||
        renderer.end_frame() != FT_ERR_SUCCESS || renderer.frame_count() != 1U ||
        renderer.shutdown() != FT_ERR_SUCCESS)
        return 23;

    cnc::OfflineNetworkSession network;
    const uint8_t payload = 0U;
    if (network.initialize() != FT_ERR_SUCCESS || network.is_online() == FT_TRUE ||
        network.connect("offline.example") != FT_ERR_INVALID_OPERATION ||
        network.send(&payload, static_cast<cnc::Size>(1U)) != FT_ERR_INVALID_OPERATION ||
        network.send_command_frame(command_frame) != FT_ERR_INVALID_OPERATION ||
        network.shutdown() != FT_ERR_SUCCESS)
        return 24;

    std::cout << "libft smoke ok (" << CNC_PROJECT_VERSION << ")\n";
    return 0;
}
