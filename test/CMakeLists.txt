add_subdirectory(gtest-1.7.0 EXCLUDE_FROM_ALL)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_string test_string.cpp)
target_link_libraries(test_string gtest gtest_main string_theory)
add_test(NAME test_string COMMAND test_string)

if(ST_HAVE_FORMAT)
    add_executable(test_format test_format.cpp)
    target_link_libraries(test_format gtest gtest_main string_theory)
    add_test(NAME test_format COMMAND test_format)
endif()

add_executable(string_profile EXCLUDE_FROM_ALL profile.cpp)
target_link_libraries(string_profile string_theory)
