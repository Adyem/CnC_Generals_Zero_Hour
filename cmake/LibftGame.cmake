# The Libft Game module is intentionally isolated while its transitive graph is
# being made portable. This manifest mirrors Game.mk but does not silently pull
# Game into the default headless product.
set(CNC_LIBFT_GAME_SOURCES
    game_map3d.cpp game_character_constructor.cpp game_character_getters_setters.cpp
    game_character_add_remove.cpp game_character_misc.cpp game_character_metrics.cpp
    game_character_save_load.cpp game_quest.cpp game_achievement.cpp
    game_reputation.cpp game_buff.cpp game_debuff.cpp game_skill.cpp game_upgrade.cpp
    game_event.cpp game_event_scheduler.cpp game_event_scheduler_telemetry.cpp
    game_world.cpp game_voxel_chunk.cpp game_voxel_region.cpp game_block_edit_op.cpp
    game_world_replay.cpp game_server.cpp game_item.cpp game_inventory.cpp
    game_equipment.cpp game_save.cpp game_load.cpp game_experience_table.cpp
    game_resistance.cpp game_pathfinding.cpp game_crafting.cpp game_data_catalog.cpp
    game_dialogue_line.cpp game_dialogue_script.cpp game_dialogue_table.cpp
    game_behavior_action.cpp game_behavior_profile.cpp game_behavior_table.cpp
    game_region_definition.cpp game_world_region.cpp game_world_registry.cpp
    game_price_definition.cpp game_rarity_band.cpp game_vendor_profile.cpp
    game_currency_rate.cpp game_economy_table.cpp game_progress_tracker.cpp
    game_hooks.cpp game_state.cpp game_behavior_tree.cpp game_lua_runtime.cpp
    game_scripting_bridge.cpp
)
set(CNC_LIBFT_GAME_RESOLVED)
foreach(source IN LISTS CNC_LIBFT_GAME_SOURCES)
    if(NOT EXISTS "${CNC_LIBFT_ROOT}/Modules/Game/${source}")
        message(FATAL_ERROR "Libft Game source is missing: ${source}")
    endif()
    list(APPEND CNC_LIBFT_GAME_RESOLVED "${CNC_LIBFT_ROOT}/Modules/Game/${source}")
endforeach()

add_library(libft_game STATIC ${CNC_LIBFT_GAME_RESOLVED})
add_library(libft::game ALIAS libft_game)
target_compile_features(libft_game PUBLIC cxx_std_17)
target_compile_definitions(libft_game PRIVATE LIBFT_INTERNAL_HEADERS)
target_include_directories(libft_game PUBLIC
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Game>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Basic>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Errno>
    $<INSTALL_INTERFACE:include/libft/Game>
    $<INSTALL_INTERFACE:include/Basic>
    $<INSTALL_INTERFACE:include/Errno>)
target_link_libraries(libft_game PUBLIC libft::basic)
set_target_properties(libft_game PROPERTIES POSITION_INDEPENDENT_CODE ON)
