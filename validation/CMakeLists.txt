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

add_executable(
  linspace
  ./check_linspace.cpp
)
target_link_libraries(
  linspace
  gtest
  gtest_main
  pthread
)
add_test(NAME test_linspace COMMAND $<TARGET_FILE:linspace>)

add_executable(parser ./check_parser.cpp)
target_include_directories(parser PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(parser PRIVATE fmt::fmt-header-only)

add_executable(grid_point ./check_class_gridpoint.cpp ../src/grid.cpp)
target_include_directories(grid_point PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
  grid_point
  gtest
  gtest_main
  pthread
)
add_test(NAME test_grid_point COMMAND $<TARGET_FILE:grid_point>)
