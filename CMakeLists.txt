cmake_minimum_required(VERSION 3.12)

project(chip C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

add_link_options("-nostartfiles" "-Wl,--export-table" "-Wl,--no-entry")

add_executable(chip src/main.cpp)

set_target_properties(chip PROPERTIES
  PREFIX ""
  SUFFIX ".wasm"
)
