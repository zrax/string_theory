find_path(Fmt_INCLUDE_DIR fmt/format.h)
find_library(Fmt_LIBRARY NAMES fmt)

mark_as_advanced(Fmt_INCLUDE_DIR Fmt_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Fmt
    REQUIRED_VARS Fmt_INCLUDE_DIR Fmt_LIBRARY
)

if(Fmt_FOUND AND NOT TARGET Fmt::Fmt)
    add_library(Fmt::Fmt UNKNOWN IMPORTED)
    set_target_properties(Fmt::Fmt PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        IMPORTED_LOCATION "${Fmt_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Fmt_INCLUDE_DIR}"
    )
endif()
