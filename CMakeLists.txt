cmake_minimum_required(VERSION 3.20)

project(CnCGeneralsZeroHour
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)

option(CNC_BUILD_LIBFT_BASIC "Build the Libft Basic module" ON)
option(CNC_BUILD_LIBFT_TIME "Build the Libft Time module (requires PThread/Compatibility)" OFF)
option(CNC_BUILD_RUNTIME "Build the Libft-native runtime composition root" ON)
option(CNC_BUILD_SIMULATION "Build the deterministic headless world scaffold" ON)
option(CNC_BUILD_GAME_DATA "Build the game-owned Zero Hour definition catalog" ON)
option(CNC_BUILD_GAME_SESSION "Build the headless game composition root" ON)
option(CNC_BUILD_DETERMINISM_TEST "Build the deterministic replay test" ON)
option(CNC_BUILD_MIGRATION_AUDIT "Build the SafeDisk/GameSpy exclusion audit" ON)
option(CNC_BUILD_RENDERER "Build the headless GPGR-facing renderer seam" ON)
option(CNC_WARNINGS_AS_ERRORS "Treat warnings as errors for migration targets" OFF)
option(CNC_BUILD_SMOKE_TEST "Build the portable Libft integration smoke test" ON)
option(CNC_BUILD_LEGACY_REFERENCE "Expose the legacy SAGE reference target (not implemented)" OFF)
option(CNC_ENABLE_MULTIPLAYER "Enable the replacement multiplayer stack (not implemented)" OFF)

if(CNC_ENABLE_MULTIPLAYER)
    message(FATAL_ERROR
        "CNC_ENABLE_MULTIPLAYER is not implemented during the offline engine migration. "
        "Keep it OFF until the Libft-native networking phase is complete.")
endif()

if(CNC_BUILD_LEGACY_REFERENCE AND NOT WIN32)
    message(FATAL_ERROR
        "CNC_BUILD_LEGACY_REFERENCE is a temporary Windows-only reference target.")
endif()

set(CNC_LIBFT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/external/libft" CACHE PATH
    "Path to the pinned Libft checkout")

if(NOT EXISTS "${CNC_LIBFT_ROOT}/FullLibft.hpp")
    message(FATAL_ERROR
        "Libft was not found at '${CNC_LIBFT_ROOT}'. "
        "Initialize the submodule with: git submodule update --init --recursive")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake")

if(CNC_BUILD_LIBFT_BASIC)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftBasic.cmake")
endif()

if(CNC_BUILD_LIBFT_TIME)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_LIBFT_TIME requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftTime.cmake")
endif()

if(CNC_BUILD_RUNTIME)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_RUNTIME requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()

    add_library(cnc_runtime STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/runtime/src/Runtime.cpp"
    )
    add_library(cnc::runtime ALIAS cnc_runtime)
    target_compile_features(cnc_runtime PUBLIC cxx_std_17)
    target_include_directories(cnc_runtime
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/engine/runtime/include"
    )
    target_link_libraries(cnc_runtime PUBLIC libft::basic)
    cnc_enable_native_warnings(cnc_runtime)
endif()

if(CNC_BUILD_SIMULATION)
    if(NOT TARGET cnc::runtime)
        message(FATAL_ERROR "CNC_BUILD_SIMULATION requires CNC_BUILD_RUNTIME=ON")
    endif()

    add_library(cnc_simulation STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/World.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/SystemRegistry.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/DefinitionRegistry.cpp"
    )
    add_library(cnc::simulation ALIAS cnc_simulation)
    target_compile_features(cnc_simulation PUBLIC cxx_std_17)
    target_include_directories(cnc_simulation
        PUBLIC
            "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/include"
    )
    target_link_libraries(cnc_simulation PUBLIC cnc::runtime)
    cnc_enable_native_warnings(cnc_simulation)
endif()

if(CNC_BUILD_GAME_DATA)
    if(NOT TARGET cnc::simulation)
        message(FATAL_ERROR "CNC_BUILD_GAME_DATA requires CNC_BUILD_SIMULATION=ON")
    endif()
    add_library(cnc_zero_hour_data STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/src/Catalog.cpp"
    )
    add_library(cnc::zero_hour_data ALIAS cnc_zero_hour_data)
    target_compile_features(cnc_zero_hour_data PUBLIC cxx_std_17)
    target_include_directories(cnc_zero_hour_data PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/include")
    target_link_libraries(cnc_zero_hour_data PUBLIC cnc::simulation)
    cnc_enable_native_warnings(cnc_zero_hour_data)
endif()

if(CNC_BUILD_GAME_SESSION)
    if(NOT TARGET cnc::zero_hour_data)
        message(FATAL_ERROR "CNC_BUILD_GAME_SESSION requires CNC_BUILD_GAME_DATA=ON")
    endif()
    add_library(cnc_game_session STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/game/src/GameSession.cpp"
    )
    add_library(cnc::game_session ALIAS cnc_game_session)
    target_compile_features(cnc_game_session PUBLIC cxx_std_17)
    target_include_directories(cnc_game_session PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/game/include")
    target_link_libraries(cnc_game_session PUBLIC cnc::zero_hour_data)
    cnc_enable_native_warnings(cnc_game_session)
endif()

if(CNC_BUILD_RENDERER)
    if(NOT TARGET cnc::runtime)
        message(FATAL_ERROR "CNC_BUILD_RENDERER requires CNC_BUILD_RUNTIME=ON")
    endif()
    add_library(cnc_render STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/render/src/Renderer.cpp")
    add_library(cnc::render ALIAS cnc_render)
    target_compile_features(cnc_render PUBLIC cxx_std_17)
    target_include_directories(cnc_render PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/render/include")
    target_link_libraries(cnc_render PUBLIC cnc::runtime)
    cnc_enable_native_warnings(cnc_render)
endif()

if(CNC_BUILD_DETERMINISM_TEST)
    if(NOT TARGET cnc::game_session)
        message(FATAL_ERROR "CNC_BUILD_DETERMINISM_TEST requires CNC_BUILD_GAME_SESSION=ON")
    endif()
    add_executable(cnc_determinism_replay
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/determinism/replay.cpp"
    )
    target_compile_features(cnc_determinism_replay PRIVATE cxx_std_17)
    target_link_libraries(cnc_determinism_replay PRIVATE cnc::game_session)
    cnc_enable_native_warnings(cnc_determinism_replay)
    if(BUILD_TESTING)
        add_test(NAME determinism.replay COMMAND cnc_determinism_replay)
        set_tests_properties(determinism.replay PROPERTIES LABELS "determinism;unit")
    endif()
endif()

if(CNC_BUILD_MIGRATION_AUDIT)
    add_custom_target(cnc_validate_migration_sources
        COMMAND ${CMAKE_COMMAND}
            -DCNC_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ValidateMigrationSources.cmake"
        VERBATIM)
    if(BUILD_TESTING)
        add_test(NAME migration.exclusions
            COMMAND ${CMAKE_COMMAND}
                -DCNC_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
                -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ValidateMigrationSources.cmake")
        set_tests_properties(migration.exclusions PROPERTIES LABELS "unit;integration-offline")
    endif()
endif()

if(CNC_BUILD_SMOKE_TEST)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_SMOKE_TEST requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()

    add_executable(cnc_libft_smoke
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke/libft_smoke.cpp"
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/zero_hour_manifest.csv"
        "${CMAKE_CURRENT_BINARY_DIR}/zero_hour_manifest.csv" COPYONLY)
    target_compile_features(cnc_libft_smoke PRIVATE cxx_std_17)
    target_link_libraries(cnc_libft_smoke PRIVATE libft::basic)
    if(TARGET cnc::runtime)
        target_link_libraries(cnc_libft_smoke PRIVATE cnc::runtime)
    endif()
    if(TARGET cnc::simulation)
        target_link_libraries(cnc_libft_smoke PRIVATE cnc::simulation)
    endif()
    if(TARGET cnc::zero_hour_data)
        target_link_libraries(cnc_libft_smoke PRIVATE cnc::zero_hour_data)
    endif()
    if(TARGET cnc::game_session)
        target_link_libraries(cnc_libft_smoke PRIVATE cnc::game_session)
    endif()
    if(TARGET cnc::render)
        target_link_libraries(cnc_libft_smoke PRIVATE cnc::render)
    endif()
    cnc_enable_native_warnings(cnc_libft_smoke)
    target_compile_definitions(cnc_libft_smoke PRIVATE
        CNC_PROJECT_VERSION="${PROJECT_VERSION}"
        CNC_ZERO_HOUR_MANIFEST_PATH="${CMAKE_CURRENT_BINARY_DIR}/zero_hour_manifest.csv"
    )

    if(BUILD_TESTING)
        add_test(NAME libft.smoke COMMAND cnc_libft_smoke)
        set_tests_properties(libft.smoke PROPERTIES LABELS "unit;portable")
    endif()
endif()

if(CNC_BUILD_LEGACY_REFERENCE)
    message(STATUS
        "Legacy SAGE reference target is reserved for a later migration phase; "
        "no legacy sources are linked by this configure yet.")
endif()

message(STATUS "CNC build profile: ${CMAKE_BUILD_TYPE}")
message(STATUS "CNC Libft root: ${CNC_LIBFT_ROOT}")
message(STATUS "CNC multiplayer: disabled (offline migration stage)")
