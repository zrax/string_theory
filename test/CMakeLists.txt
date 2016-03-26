include_directories(${GTEST_INCLUDE_DIR})
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(test_string test_string.cpp)
target_link_libraries(test_string gtest gtest_main string_theory)
add_test(NAME test_string COMMAND test_string)
add_dependencies(check test_string)

if(ST_HAVE_FORMAT)
    add_executable(test_format test_format.cpp)
    target_link_libraries(test_format gtest gtest_main string_theory)
    add_test(NAME test_format COMMAND test_format)
    add_dependencies(check test_format)
endif()
