if(TARGET libft::basic)
    return()
endif()

set(_cnc_libft_basic_dir "${CNC_LIBFT_ROOT}/Modules/Basic")

set(_cnc_libft_basic_sources
    "${_cnc_libft_basic_dir}/basic_alignment.cpp"
    "${_cnc_libft_basic_dir}/basic_ascii_classification.cpp"
    "${_cnc_libft_basic_dir}/basic_ascii_utf8_helpers.cpp"
    "${_cnc_libft_basic_dir}/basic_atoi.cpp"
    "${_cnc_libft_basic_dir}/basic_atol.cpp"
    "${_cnc_libft_basic_dir}/basic_bzero.cpp"
    "${_cnc_libft_basic_dir}/basic_checked_arithmetic.cpp"
    "${_cnc_libft_basic_dir}/basic_constant_time_compare.cpp"
    "${_cnc_libft_basic_dir}/basic_isalnum.cpp"
    "${_cnc_libft_basic_dir}/basic_isalpha.cpp"
    "${_cnc_libft_basic_dir}/basic_isdigit.cpp"
    "${_cnc_libft_basic_dir}/basic_islower.cpp"
    "${_cnc_libft_basic_dir}/basic_isprint.cpp"
    "${_cnc_libft_basic_dir}/basic_isspace.cpp"
    "${_cnc_libft_basic_dir}/basic_isupper.cpp"
    # basic_locale_compare.cpp depends on System_utils; it will be added when
    # the corresponding Libft target is integrated.
    "${_cnc_libft_basic_dir}/basic_memchr.cpp"
    "${_cnc_libft_basic_dir}/basic_memcmp.cpp"
    "${_cnc_libft_basic_dir}/basic_memcpy.cpp"
    "${_cnc_libft_basic_dir}/basic_memcpy_s.cpp"
    "${_cnc_libft_basic_dir}/basic_memmem.cpp"
    "${_cnc_libft_basic_dir}/basic_memmove.cpp"
    "${_cnc_libft_basic_dir}/basic_memmove_s.cpp"
    "${_cnc_libft_basic_dir}/basic_memrchr.cpp"
    "${_cnc_libft_basic_dir}/basic_memset.cpp"
    "${_cnc_libft_basic_dir}/basic_memswap.cpp"
    "${_cnc_libft_basic_dir}/basic_nullptr.cpp"
    "${_cnc_libft_basic_dir}/basic_parse_fixed.cpp"
    "${_cnc_libft_basic_dir}/basic_strcat_s.cpp"
    "${_cnc_libft_basic_dir}/basic_strchr.cpp"
    "${_cnc_libft_basic_dir}/basic_strcmp.cpp"
    "${_cnc_libft_basic_dir}/basic_strcpy_s.cpp"
    "${_cnc_libft_basic_dir}/basic_string_predicates.cpp"
    "${_cnc_libft_basic_dir}/basic_string_trim.cpp"
    "${_cnc_libft_basic_dir}/basic_striteri.cpp"
    "${_cnc_libft_basic_dir}/basic_strlcat.cpp"
    "${_cnc_libft_basic_dir}/basic_strlcpy.cpp"
    "${_cnc_libft_basic_dir}/basic_strncat_s.cpp"
    "${_cnc_libft_basic_dir}/basic_strncmp.cpp"
    "${_cnc_libft_basic_dir}/basic_strncpy.cpp"
    "${_cnc_libft_basic_dir}/basic_strncpy_s.cpp"
    "${_cnc_libft_basic_dir}/basic_strnlen.cpp"
    "${_cnc_libft_basic_dir}/basic_strnstr.cpp"
    "${_cnc_libft_basic_dir}/basic_strrchr.cpp"
    "${_cnc_libft_basic_dir}/basic_strstr.cpp"
    "${_cnc_libft_basic_dir}/basic_strtok.cpp"
    "${_cnc_libft_basic_dir}/basic_strtol.cpp"
    "${_cnc_libft_basic_dir}/basic_strtoul.cpp"
    "${_cnc_libft_basic_dir}/basic_tolower.cpp"
    "${_cnc_libft_basic_dir}/basic_toupper.cpp"
    "${_cnc_libft_basic_dir}/basic_utf8_decode.cpp"
    "${_cnc_libft_basic_dir}/basic_utf8_encode.cpp"
    "${_cnc_libft_basic_dir}/basic_utf8_length.cpp"
    "${_cnc_libft_basic_dir}/basic_utf8_validate.cpp"
    "${_cnc_libft_basic_dir}/basic_validate_int.cpp"
    "${_cnc_libft_basic_dir}/basic_wstrlen.cpp"
)

foreach(_source IN LISTS _cnc_libft_basic_sources)
    if(NOT EXISTS "${_source}")
        message(FATAL_ERROR "Pinned Libft checkout is missing Basic source: ${_source}")
    endif()
endforeach()

add_library(libft_basic STATIC ${_cnc_libft_basic_sources})
add_library(libft::basic ALIAS libft_basic)

target_compile_features(libft_basic PUBLIC cxx_std_17)
target_include_directories(libft_basic
    PUBLIC
        "${_cnc_libft_basic_dir}"
        "${CNC_LIBFT_ROOT}/Modules/Errno"
)

set_target_properties(libft_basic PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)
