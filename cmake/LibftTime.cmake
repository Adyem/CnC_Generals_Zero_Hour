# Keep Libft clocks opt-in until the wider platform dependency graph is wired.
set(CNC_LIBFT_TIME_SOURCES
    time_active_clock.cpp time_now.cpp time_now_ms.cpp time_monotonic.cpp
    time_monotonic_point.cpp time_monotonic_point_thread_safety.cpp
    time_duration.cpp time_duration_thread_safety.cpp time_info_thread_safety.cpp
    time_local.cpp time_sleep.cpp time_sleep_ms.cpp time_sleep_async.cpp
    time_fps.cpp time_timer.cpp time_strftime.cpp time_format.cpp time_relative.cpp
    time_relative_format.cpp time_interval.cpp time_parse.cpp time_high_resolution.cpp
    time_monotonic_translate.cpp time_timezone.cpp time_benchmark.cpp time_trace.cpp
    time_basic.cpp
)
set(CNC_LIBFT_TIME_RESOLVED)
foreach(source IN LISTS CNC_LIBFT_TIME_SOURCES)
    if(NOT EXISTS "${CNC_LIBFT_ROOT}/Modules/Time/${source}")
        message(FATAL_ERROR "Libft Time source is missing: ${source}")
    endif()
    list(APPEND CNC_LIBFT_TIME_RESOLVED "${CNC_LIBFT_ROOT}/Modules/Time/${source}")
endforeach()
add_library(libft_time STATIC ${CNC_LIBFT_TIME_RESOLVED})
add_library(libft::time ALIAS libft_time)
target_compile_features(libft_time PUBLIC cxx_std_17)
target_include_directories(libft_time PUBLIC
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Time>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Basic>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Errno>
    $<INSTALL_INTERFACE:include/libft/Time>
    $<INSTALL_INTERFACE:include/Basic>
    $<INSTALL_INTERFACE:include/Errno>)
target_link_libraries(libft_time PUBLIC libft::basic)
set_target_properties(libft_time PROPERTIES POSITION_INDEPENDENT_CODE ON)
