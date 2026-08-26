cmake_minimum_required(VERSION 3.20)

project(CnCGeneralsZeroHour
    VERSION 0.1.0
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)

option(CNC_BUILD_LIBFT_BASIC "Build the Libft Basic module" ON)
option(CNC_BUILD_LIBFT_TIME "Build the Libft Time module (requires PThread/Compatibility)" OFF)
option(CNC_BUILD_LIBFT_FILE "Build the Libft File module (requires utility graph)" OFF)
option(CNC_BUILD_LIBFT_GAME "Build the Libft Game module (requires full transitive graph)" OFF)
option(CNC_BUILD_RUNTIME "Build the Libft-native runtime composition root" ON)
option(CNC_BUILD_SIMULATION "Build the deterministic headless world scaffold" ON)
option(CNC_BUILD_GAME_DATA "Build the game-owned Zero Hour definition catalog" ON)
option(CNC_BUILD_GAME_SESSION "Build the headless game composition root" ON)
option(CNC_BUILD_DETERMINISM_TEST "Build the deterministic replay test" ON)
option(CNC_BUILD_MIGRATION_AUDIT "Build the SafeDisk/GameSpy exclusion audit" ON)
option(CNC_BUILD_RENDERER "Build the headless GPGR-facing renderer seam" ON)
option(CNC_BUILD_OFFLINE_APP "Build the offline Zero Hour headless executable" ON)
option(CNC_BUILD_NETWORK_STUB "Build the explicit offline multiplayer capability stub" ON)
option(CNC_WARNINGS_AS_ERRORS "Treat warnings as errors for migration targets" OFF)
option(CNC_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
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
set(CNC_GAME_DATA_ROOT "" CACHE PATH
    "Legally owned Generals/Zero Hour data root used only by stage_zero_hour")

if(NOT EXISTS "${CNC_LIBFT_ROOT}/FullLibft.hpp")
    message(FATAL_ERROR
        "Libft was not found at '${CNC_LIBFT_ROOT}'. "
        "Initialize the submodule with: git submodule update --init --recursive")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake")

set(CNC_GIT_COMMIT "unknown")
execute_process(
    COMMAND git rev-parse --short=12 HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE CNC_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET)
if(CNC_GIT_COMMIT STREQUAL "")
    set(CNC_GIT_COMMIT "unknown")
endif()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated/CncBuild")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/BuildInfo.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/CncBuild/BuildInfo.hpp" @ONLY)

if(CNC_BUILD_LIBFT_BASIC)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftBasic.cmake")
endif()

if(CNC_BUILD_LIBFT_TIME)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_LIBFT_TIME requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftTime.cmake")
endif()

if(CNC_BUILD_LIBFT_FILE)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_LIBFT_FILE requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftFile.cmake")
endif()

if(CNC_BUILD_LIBFT_GAME)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_LIBFT_GAME requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftGame.cmake")
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
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/runtime/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(cnc_runtime PUBLIC libft::basic)
    cnc_enable_native_warnings(cnc_runtime)
    cnc_enable_native_sanitizers(cnc_runtime)
endif()

if(CNC_BUILD_SIMULATION)
    if(NOT TARGET cnc::runtime)
        message(FATAL_ERROR "CNC_BUILD_SIMULATION requires CNC_BUILD_RUNTIME=ON")
    endif()

    add_library(cnc_simulation STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/World.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/SnapshotCodec.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/CommandCodec.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/SystemRegistry.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/src/DefinitionRegistry.cpp"
    )
    add_library(cnc::simulation ALIAS cnc_simulation)
    target_compile_features(cnc_simulation PUBLIC cxx_std_17)
    target_include_directories(cnc_simulation
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/simulation/include>
            $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(cnc_simulation PUBLIC cnc::runtime)
    cnc_enable_native_warnings(cnc_simulation)
    cnc_enable_native_sanitizers(cnc_simulation)
endif()

if(CNC_BUILD_GAME_DATA)
    if(NOT TARGET cnc::simulation)
        message(FATAL_ERROR "CNC_BUILD_GAME_DATA requires CNC_BUILD_SIMULATION=ON")
    endif()
    add_library(cnc_zero_hour_data STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/src/Catalog.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/src/ScienceLedger.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/src/SpecialPowerLedger.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/src/GeneralRoster.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/src/PlayerState.cpp"
    )
    add_library(cnc::zero_hour_data ALIAS cnc_zero_hour_data)
    target_compile_features(cnc_zero_hour_data PUBLIC cxx_std_17)
    target_include_directories(cnc_zero_hour_data PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/games/zero_hour/include>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(cnc_zero_hour_data PUBLIC cnc::simulation)
    cnc_enable_native_warnings(cnc_zero_hour_data)
    cnc_enable_native_sanitizers(cnc_zero_hour_data)
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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/game/include>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(cnc_game_session PUBLIC cnc::zero_hour_data)
    if(TARGET cnc::render)
        target_link_libraries(cnc_game_session PUBLIC cnc::render)
    endif()
    if(TARGET cnc::network_stub)
        target_link_libraries(cnc_game_session PUBLIC cnc::network_stub)
    endif()
    cnc_enable_native_warnings(cnc_game_session)
    cnc_enable_native_sanitizers(cnc_game_session)
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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/render/include>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(cnc_render PUBLIC cnc::runtime cnc::simulation)
    cnc_enable_native_warnings(cnc_render)
    cnc_enable_native_sanitizers(cnc_render)
endif()

if(CNC_BUILD_NETWORK_STUB)
    if(NOT TARGET cnc::runtime)
        message(FATAL_ERROR "CNC_BUILD_NETWORK_STUB requires CNC_BUILD_RUNTIME=ON")
    endif()
    add_library(cnc_network_stub STATIC
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/network/src/NetworkSession.cpp")
    add_library(cnc::network_stub ALIAS cnc_network_stub)
    target_compile_features(cnc_network_stub PUBLIC cxx_std_17)
    target_include_directories(cnc_network_stub PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/network/include>
        $<INSTALL_INTERFACE:include>)
    target_link_libraries(cnc_network_stub PUBLIC cnc::runtime)
    cnc_enable_native_warnings(cnc_network_stub)
    cnc_enable_native_sanitizers(cnc_network_stub)
endif()

# Renderer/network targets are declared after the game target so their optional
# link edges are completed once both capability targets exist.
if(TARGET cnc_game_session)
    if(TARGET cnc::render)
        target_link_libraries(cnc_game_session PUBLIC cnc::render)
    endif()
    if(TARGET cnc::network_stub)
        target_link_libraries(cnc_game_session PUBLIC cnc::network_stub)
    endif()
endif()

if(CNC_BUILD_OFFLINE_APP)
    if(NOT TARGET cnc::game_session OR NOT TARGET cnc::render)
        message(FATAL_ERROR "CNC_BUILD_OFFLINE_APP requires game_session and render targets")
    endif()
    add_executable(cnc_zero_hour_headless
        "${CMAKE_CURRENT_SOURCE_DIR}/apps/zero_hour_headless/main.cpp")
    target_compile_features(cnc_zero_hour_headless PRIVATE cxx_std_17)
    target_link_libraries(cnc_zero_hour_headless PRIVATE cnc::game_session cnc::render)
    cnc_enable_native_warnings(cnc_zero_hour_headless)
    cnc_enable_native_sanitizers(cnc_zero_hour_headless)
    if(BUILD_TESTING)
        add_test(NAME integration.offline COMMAND cnc_zero_hour_headless)
        set_tests_properties(integration.offline PROPERTIES LABELS "integration-offline;unit")
    endif()
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
    cnc_enable_native_sanitizers(cnc_determinism_replay)
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
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/zero_hour_invalid_manifest.csv"
        "${CMAKE_CURRENT_BINARY_DIR}/zero_hour_invalid_manifest.csv" COPYONLY)
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
    if(TARGET cnc::network_stub)
        target_link_libraries(cnc_libft_smoke PRIVATE cnc::network_stub)
    endif()
    cnc_enable_native_warnings(cnc_libft_smoke)
    cnc_enable_native_sanitizers(cnc_libft_smoke)
    target_compile_definitions(cnc_libft_smoke PRIVATE
        CNC_PROJECT_VERSION="${PROJECT_VERSION}"
        CNC_ZERO_HOUR_MANIFEST_PATH="${CMAKE_CURRENT_BINARY_DIR}/zero_hour_manifest.csv"
        CNC_ZERO_HOUR_INVALID_MANIFEST_PATH="${CMAKE_CURRENT_BINARY_DIR}/zero_hour_invalid_manifest.csv"
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

# Installation is target-scoped and never copies the legacy Run/ tree. The
# optional offline package contains only project-owned native artifacts.
if(TARGET libft_basic)
    set_target_properties(libft_basic PROPERTIES EXPORT_NAME basic)
    install(TARGETS libft_basic EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY external/libft/Modules/Basic/ DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/Basic" FILES_MATCHING PATTERN "*.hpp")
    install(DIRECTORY external/libft/Modules/Errno/ DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/Errno" FILES_MATCHING PATTERN "*.hpp")
endif()
if(TARGET libft_time)
    set_target_properties(libft_time PROPERTIES EXPORT_NAME time)
    install(TARGETS libft_time EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY external/libft/Modules/Time/ DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/libft/Time" FILES_MATCHING PATTERN "*.hpp")
endif()
if(TARGET libft_game)
    set_target_properties(libft_game PROPERTIES EXPORT_NAME game)
    install(TARGETS libft_game EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY external/libft/Modules/Game/ DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/libft/Game" FILES_MATCHING PATTERN "*.hpp")
endif()
if(TARGET libft_file)
    set_target_properties(libft_file PROPERTIES EXPORT_NAME file)
    install(TARGETS libft_file EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY external/libft/Modules/File/ DESTINATION
        "${CMAKE_INSTALL_INCLUDEDIR}/libft/File" FILES_MATCHING PATTERN "*.hpp")
endif()
if(TARGET cnc_runtime)
    set_target_properties(cnc_runtime PROPERTIES EXPORT_NAME runtime)
    install(TARGETS cnc_runtime EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY engine/runtime/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/CncBuild/BuildInfo.hpp"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/CncBuild")
endif()
if(TARGET cnc_simulation)
    set_target_properties(cnc_simulation PROPERTIES EXPORT_NAME simulation)
    install(TARGETS cnc_simulation EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY engine/simulation/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(TARGET cnc_zero_hour_data)
    set_target_properties(cnc_zero_hour_data PROPERTIES EXPORT_NAME zero_hour_data)
    install(TARGETS cnc_zero_hour_data EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY games/zero_hour/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(TARGET cnc_game_session)
    set_target_properties(cnc_game_session PROPERTIES EXPORT_NAME game_session)
    install(TARGETS cnc_game_session EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY engine/game/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(TARGET cnc_render)
    set_target_properties(cnc_render PROPERTIES EXPORT_NAME render)
    install(TARGETS cnc_render EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY engine/render/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(TARGET cnc_network_stub)
    set_target_properties(cnc_network_stub PROPERTIES EXPORT_NAME network_stub)
    install(TARGETS cnc_network_stub EXPORT cncTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(DIRECTORY engine/network/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
if(TARGET cnc_zero_hour_headless)
    set_target_properties(cnc_zero_hour_headless PROPERTIES EXPORT_NAME zero_hour_headless)
    install(TARGETS cnc_zero_hour_headless EXPORT cncTargets RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
    add_custom_target(stage_zero_hour
        COMMAND ${CMAKE_COMMAND}
            -DCNC_GAME_DATA_ROOT=${CNC_GAME_DATA_ROOT}
            -DCNC_STAGE_ROOT=${CMAKE_BINARY_DIR}/stage/zero_hour
            -DCNC_EXECUTABLE=$<TARGET_FILE:cnc_zero_hour_headless>
            -DCNC_PROJECT_MANIFEST=${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/zero_hour_manifest.csv
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/StageZeroHour.cmake"
        DEPENDS cnc_zero_hour_headless
        VERBATIM)
endif()

install(EXPORT cncTargets
    NAMESPACE cnc::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CnCGeneralsZeroHour")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CnCGeneralsZeroHourConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CnCGeneralsZeroHourConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CnCGeneralsZeroHour")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CnCGeneralsZeroHourConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CnCGeneralsZeroHourConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CnCGeneralsZeroHourConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/CnCGeneralsZeroHour")

set(CPACK_PACKAGE_NAME "cnc-generals-zero-hour")
set(CPACK_PACKAGE_VENDOR "CnC Generals Zero Hour contributors")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "ZIP;TGZ")
include(CPack)
