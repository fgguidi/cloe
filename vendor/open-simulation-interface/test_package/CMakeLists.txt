cmake_minimum_required(VERSION 3.15...3.27 FATAL_ERROR)
project(test_package CXX)

find_package(open_simulation_interface REQUIRED QUIET CONFIG)

add_executable(${PROJECT_NAME} test_package.cpp)
target_link_libraries(${PROJECT_NAME}
  open_simulation_interface::open_simulation_interface
  )
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)

