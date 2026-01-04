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
