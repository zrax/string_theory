# Copyright (c) 2016 Michael Hansen
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Why is this OFF by default?  It doesn't work with CMake's defaults, and
# should really only be off if the project has specifically been set up to
# use the static MSVC CRT
set(gtest_force_shared_crt ON CACHE INTERNAL "Override gtest default")

# Force other gtest settings to line up correclty -- we're compiling
# string_theory, not gtest...
set(gtest_build_samples OFF CACHE INTERNAL "Override gtest default")
set(gtest_build_tests OFF CACHE INTERNAL "Override gtest default")
set(gtest_disable_pthreads OFF CACHE INTERNAL "Override gtest default")
set(gtest_hide_internal_symbols ON CACHE INTERNAL "Override gtest default")

add_subdirectory(gtest-1.8.0 EXCLUDE_FROM_ALL)

add_executable(st_gtests "")
target_link_libraries(st_gtests PRIVATE gtest gtest_main string_theory)
target_include_directories(st_gtests PRIVATE
    ${GTEST_INCLUDE_DIR}
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
)

if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET st_gtests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gtest>"
        "$<TARGET_FILE_DIR:st_gtests>"
    )
    add_custom_command(TARGET st_gtests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gtest_main>"
        "$<TARGET_FILE_DIR:st_gtests>"
    )
endif()

target_sources(st_gtests PRIVATE
    test_buffer.cpp
    test_string.cpp
    test_codecs.cpp
    test_iostream.cpp
    test_sstream.cpp
    test_format.cpp
    test_stdio.cpp
    test_regress.cpp
)

if(WIN32)
    target_sources(st_gtests PRIVATE test_winheaders.cpp)
endif()

option(ST_BUILD_PROFILE_TEST "Build string profile test application" OFF)
if(ST_BUILD_PROFILE_TEST)
    find_package(Boost)
    if(Boost_FOUND)
        add_definitions(-DST_PROFILE_HAVE_BOOST)
        include_directories(${Boost_INCLUDE_DIRS})
    endif()

    find_package(Qt5Core)
    if(Qt5Core_FOUND)
        add_definitions(-DST_PROFILE_HAVE_QSTRING)
    endif()

    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(GLIBMM glibmm-2.4)
        if(GLIBMM_FOUND)
            add_definitions(-DST_PROFILE_HAVE_GLIBMM)
            include_directories(${GLIBMM_INCLUDE_DIRS})
        endif()
    endif()

    find_package(Fmt)
    if(Fmt_FOUND)
        add_definitions(-DST_PROFILE_HAVE_FMT)
    endif()

    add_executable(string_profile EXCLUDE_FROM_ALL profile.cpp)
    target_link_libraries(string_profile string_theory)
    if(Qt5Core_FOUND)
        target_link_libraries(string_profile Qt5::Core)
    endif()
    if(GLIBMM_FOUND)
        target_link_libraries(string_profile ${GLIBMM_LIBRARIES})
    endif()
    if(Fmt_FOUND)
        target_link_libraries(string_profile Fmt::Fmt)
    endif()
endif()

cmake_policy(PUSH)
if(${CMAKE_VERSION} VERSION_LESS "3.11" AND POLICY CMP0037)
    # The "test" target is reserved when ctest is included (CMake 3.11+),
    # but older versions of CMake will reserve it always, so we must
    # explicitly allow older CMake versions to allow the target name
    cmake_policy(SET CMP0037 OLD)
endif()
add_custom_target(test "$<TARGET_FILE:st_gtests>")
cmake_policy(POP)

if(ST_BUILD_TEST_COVERAGE)
    setup_target_for_coverage(test_coverage
            "${CMAKE_COMMAND} --build \"${CMAKE_BINARY_DIR}\" --target test"
            coverage)
endif()
