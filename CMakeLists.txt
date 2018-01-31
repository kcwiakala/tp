cmake_minimum_required (VERSION 2.6)

project(tp)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE test_sources "test/*.cpp")

add_executable(tp ${test_sources})
target_include_directories(tp PUBLIC "test")
target_link_libraries(tp PUBLIC gtest gmock pthread)