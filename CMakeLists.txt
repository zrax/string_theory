# This file is part of string_theory.
#
# string_theory is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# string_theory is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with string_theory.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8)
project(string_theory)

set(ST_MAJOR_VERSION 1)
set(ST_MINOR_VERSION 0)
set(ST_VERSION ${ST_MAJOR_VERSION}.${ST_MINOR_VERSION})

if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_COMPILER_ID}" MATCHES ".*Clang")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")

    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" HAVE_STD_CXX11)
    check_cxx_compiler_flag("-std=c++0x" HAVE_STD_CXX0X)
    if(HAVE_STD_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    elseif(HAVE_STD_CXX0X)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    endif()

    if(APPLE AND (HAVE_STD_CXX11 OR HAVE_STD_CXX0X))
        set(CMAKE_CXX_FLAGS "-stdlib=libc++")
    endif()
endif()

try_compile(ST_HAVE_NOEXCEPT "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_noexcept.cpp")
try_compile(ST_HAVE_OVERRIDE "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_override.cpp")
try_compile(ST_HAVE_STATIC_ASSERT "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_static_assert.cpp")
try_compile(ST_HAVE_RVALUE_MOVE "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_rvalue_move.cpp")
try_compile(ST_HAVE_CHAR_TYPES "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_char_types.cpp")
try_compile(ST_HAVE_INT64 "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_int64.cpp")
try_compile(ST_HAVE_DELETED_FUNC "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_deleted_func.cpp")
try_compile(ST_HAVE_VARARG_TEMPLATE "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_vararg_template.cpp")
try_compile(ST_HAVE_TYPE_TRAITS "${PROJECT_BINARY_DIR}"
            "${PROJECT_SOURCE_DIR}/cmake/check_type_traits.cpp")

include(CheckTypeSize)
if(NOT ST_WCHAR_BYTES)
    check_type_size("wchar_t" ST_WCHAR_BYTES)
    if(NOT ST_WCHAR_BYTES)
        message(FATAL_ERROR "Could not determine sizeof(wchar_t)")
        set(ST_WCHAR_BYTES 0)
    endif()
endif()

if(NOT ST_SIZET_BYTES)
    check_type_size("size_t" ST_SIZET_BYTES)
    if(NOT ST_SIZET_BYTES)
        message(FATAL_ERROR "Could not determine sizeof(size_t)")
        set(ST_SIZET_BYTES 0)
    endif()
endif()

add_definitions(-DST_BUILD_EXPORTS)

configure_file("${PROJECT_SOURCE_DIR}/include/st_config.h.in"
               "${PROJECT_BINARY_DIR}/include/st_config.h")
include_directories("${PROJECT_BINARY_DIR}/include")

set(ST_HEADERS
    include/st_assert.h
    include/st_charbuffer.h
    include/st_string.h
    include/st_stringstream.h
    "${PROJECT_BINARY_DIR}/include/st_config.h"
)
include_directories("${PROJECT_SOURCE_DIR}/include")

set(ST_SOURCES
    src/st_assert.cpp
    src/st_charbuffer.cpp
    src/st_string.cpp
    src/st_stringstream.cpp
)

if(ST_HAVE_VARARG_TEMPLATE AND ST_HAVE_TYPE_TRAITS)
    set(ST_HEADERS ${ST_HEADERS}
        include/st_formatter.h
        include/st_format.h
        include/st_stlformatter.h
    )
    set(ST_SOURCES ${ST_SOURCES}
        src/st_format.cpp
        src/st_stlformatter.cpp
    )
    set(ST_HAVE_FORMAT TRUE)
endif()

set(ST_INSTALL_BIN_DIR "bin" CACHE PATH "Path to install DLLs on Windows")
set(ST_INSTALL_LIB_DIR "lib" CACHE PATH "Path to install shared libraries")
set(ST_INSTALL_INCLUDE_DIR "include" CACHE PATH "Path to install headers")
set(ST_INSTALL_CMAKE_DIR "lib/cmake" CACHE PATH "Path to install CMake files")

foreach(path LIB INCLUDE CMAKE)
    set(var ST_INSTALL_${path}_DIR)
    if(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()

option(ST_BUILD_STATIC "Build string_theory as a static library" OFF)
if(ST_BUILD_STATIC)
    set(ST_LIB_TYPE STATIC)
else()
    set(ST_LIB_TYPE SHARED)
endif()

add_library(string_theory ${ST_LIB_TYPE}
            ${ST_HEADERS} ${ST_SOURCES})

set_target_properties(string_theory PROPERTIES
    VERSION ${ST_VERSION}
    SOVERSION ${ST_MAJOR_VERSION}
)

install(TARGETS string_theory
        EXPORT string_theory-targets
        RUNTIME DESTINATION "${ST_INSTALL_BIN_DIR}" COMPONENT shlib
        LIBRARY DESTINATION "${ST_INSTALL_LIB_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${ST_INSTALL_LIB_DIR}" COMPONENT shlib
)
install(FILES ${ST_HEADERS}
        DESTINATION "${ST_INSTALL_INCLUDE_DIR}/string_theory" COMPONENT dev)

export(TARGETS string_theory
       FILE "${PROJECT_BINARY_DIR}/string_theory-targets.cmake")
export(PACKAGE string_theory)

file(RELATIVE_PATH REL_INCLUDE_DIR "${ST_INSTALL_CMAKE_DIR}"
     "${ST_INSTALL_INCLUDE_DIR}")
set(ST_CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(cmake/string_theory-config.cmake.in
               "${PROJECT_BINARY_DIR}/string_theory-config.cmake" @ONLY)
set(ST_CONF_INCLUDE_DIRS "\${STRING_THEORY_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(cmake/string_theory-config.cmake.in
               "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/string_theory-config.cmake" @ONLY)
configure_file(cmake/string_theory-config-version.cmake.in
               "${PROJECT_BINARY_DIR}/string_theory-config-version.cmake" @ONLY)

install(FILES
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/string_theory-config.cmake"
        "${PROJECT_BINARY_DIR}/string_theory-config-version.cmake"
        DESTINATION "${ST_INSTALL_CMAKE_DIR}" COMPONENT dev)
install(EXPORT string_theory-targets
        DESTINATION "${ST_INSTALL_CMAKE_DIR}" COMPONENT dev)

enable_testing()
add_subdirectory(test EXCLUDE_FROM_ALL)
