# Opt-in Libft File target. File depends on several utility modules, so it
# remains isolated until those modules have portable CMake targets.
set(CNC_LIBFT_FILE_SOURCES
    file_opendir.cpp file_check_directory.cpp file_mkdir.cpp file_copy.cpp
    file_move.cpp file_exists.cpp file_delete.cpp file_path_join.cpp
    file_path_relative.cpp file_path_normalize.cpp file_path_parts.cpp
    file_watch.cpp file_fopen.cpp file_fclose.cpp file_fgets.cpp
    file_list_directory.cpp file_read_write.cpp file_recursive.cpp file_hash.cpp
    file_metadata.cpp file_security.cpp file_status.cpp)
set(CNC_LIBFT_FILE_RESOLVED)
foreach(source IN LISTS CNC_LIBFT_FILE_SOURCES)
    if(NOT EXISTS "${CNC_LIBFT_ROOT}/Modules/File/${source}")
        message(FATAL_ERROR "Libft File source is missing: ${source}")
    endif()
    list(APPEND CNC_LIBFT_FILE_RESOLVED "${CNC_LIBFT_ROOT}/Modules/File/${source}")
endforeach()
add_library(libft_file STATIC ${CNC_LIBFT_FILE_RESOLVED})
add_library(libft::file ALIAS libft_file)
target_compile_features(libft_file PUBLIC cxx_std_17)
target_compile_definitions(libft_file PRIVATE LIBFT_INTERNAL_HEADERS)
target_include_directories(libft_file PUBLIC
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/File>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Basic>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Errno>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/CMA>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/CPP_class>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/Observability>
    $<BUILD_INTERFACE:${CNC_LIBFT_ROOT}/Modules/PThread>
    $<INSTALL_INTERFACE:include/libft/File>
    $<INSTALL_INTERFACE:include/Basic>
    $<INSTALL_INTERFACE:include/Errno>)
target_link_libraries(libft_file PUBLIC libft::basic)
set_target_properties(libft_file PROPERTIES POSITION_INDEPENDENT_CODE ON)
