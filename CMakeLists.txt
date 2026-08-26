cmake_minimum_required(VERSION 3.20)

project(CnCGeneralsZeroHour
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)

option(CNC_BUILD_LIBFT_BASIC "Build the Libft Basic module" ON)
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

if(CNC_BUILD_LIBFT_BASIC)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/LibftBasic.cmake")
endif()

if(CNC_BUILD_SMOKE_TEST)
    if(NOT TARGET libft::basic)
        message(FATAL_ERROR "CNC_BUILD_SMOKE_TEST requires CNC_BUILD_LIBFT_BASIC=ON")
    endif()

    add_executable(cnc_libft_smoke
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/smoke/libft_smoke.cpp"
    )
    target_compile_features(cnc_libft_smoke PRIVATE cxx_std_17)
    target_link_libraries(cnc_libft_smoke PRIVATE libft::basic)
    target_compile_definitions(cnc_libft_smoke PRIVATE
        CNC_PROJECT_VERSION="${PROJECT_VERSION}"
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
