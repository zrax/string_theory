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
if(ST_BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Override gtest default")
else()
    set(BUILD_SHARED_LIBS ON  CACHE INTERNAL "Override gtest default")
endif()

add_subdirectory(gtest-1.8.0 EXCLUDE_FROM_ALL)

include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

function(add_gtest _test_name)
    add_executable(${_test_name} "${_test_name}.cpp")
    target_link_libraries(${_test_name} gtest gtest_main string_theory)
    add_test(NAME ${_test_name} COMMAND ${_test_name})

    if(WIN32 AND NOT ST_BUILD_STATIC)
        add_custom_command(TARGET ${_test_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:string_theory>"
            "$<TARGET_FILE_DIR:${_test_name}>"
        )
        add_custom_command(TARGET ${_test_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:gtest>"
            "$<TARGET_FILE_DIR:${_test_name}>"
        )
        add_custom_command(TARGET ${_test_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:gtest_main>"
            "$<TARGET_FILE_DIR:${_test_name}>"
        )
    endif()
endfunction()

if (MSVC AND MSVC_VERSION EQUAL 1700)
  add_definitions(/D _VARIADIC_MAX=10)
endif()

add_gtest(test_string)
add_gtest(test_codecs)

if(ST_HAVE_FORMAT)
    add_gtest(test_format)

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
        add_executable(string_profile EXCLUDE_FROM_ALL profile.cpp)
        target_link_libraries(string_profile string_theory)
        if(Qt5Core_FOUND)
            target_link_libraries(string_profile Qt5::Core)
        endif()
    endif()
endif()
