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

add_subdirectory(gtest-1.7.0 EXCLUDE_FROM_ALL)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_string test_string.cpp)
target_link_libraries(test_string gtest gtest_main string_theory)
add_test(NAME test_string COMMAND test_string)

add_executable(test_codecs test_codecs.cpp)
target_link_libraries(test_codecs gtest gtest_main string_theory)
add_test(NAME test_codecs COMMAND test_codecs)

if(ST_HAVE_FORMAT)
    add_executable(test_format test_format.cpp)
    target_link_libraries(test_format gtest gtest_main string_theory)
    add_test(NAME test_format COMMAND test_format)

    find_package(Boost)
    if(Boost_FOUND)
        add_definitions(-DST_PROFILE_HAVE_BOOST)
        include_directories(${Boost_INCLUDE_DIRS})
    endif()
    add_executable(string_profile EXCLUDE_FROM_ALL profile.cpp)
    target_link_libraries(string_profile string_theory)
endif()
