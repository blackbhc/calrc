project(test_gtest)

add_executable(
  a_basic_test
  ./gtest_hello_world.cpp
)
target_link_libraries(
  a_basic_test
  gtest
  pthread
)

enable_testing()
add_test(NAME test_name COMMAND $<TARGET_FILE:a_basic_test>)

add_executable(parser ./check_parser.cpp)
target_include_directories(parser PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(parser PRIVATE fmt::fmt-header-only )
