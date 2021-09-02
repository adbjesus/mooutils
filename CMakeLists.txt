cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED PROJECT_NAME)
  set(MOCO_NOT_SUBPROJECT ON)
endif()

project(MOCO
  VERSION 0.1.0
  DESCRIPTION "Library and solver for Multi-Objective Knapsack Problems"
  LANGUAGES CXX
)

# Project options
option(MOCO_BUILD_SOLVER "Build the solver." ON)
option(MOCO_BUILD_TESTS "Build the tests." ON)
option(MOCO_BUILD_DOCS "Build the tests." ON)

# Extra stuff if this is the main project
if(MOCO_NOT_SUBPROJECT)
  # Turn off extensions
  set(CMAKE_CXX_EXTENSIONS OFF)
  # Create compile commands json for clang tools
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  # Include testing
  include(CTest)
endif()

# Define library target
add_library(moco INTERFACE)
target_compile_features(moco INTERFACE cxx_std_20)
target_include_directories(moco INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if(MOCO_GATHER_ANYTIME_DATA)
  target_compile_definitions(moco INTERFACE MOCO_GATHER_ANYTIME_DATA)
endif()

# Define solver target
if(MOCO_BUILD_SOLVER)
  # add_executable(mocosol apps/mocosol.cpp)
  # target_compile_features(mocosol PRIVATE cxx_std_17)
  # target_link_libraries(mocosol moco)
endif()

# Define tests target
if(BUILD_TESTING AND MOCO_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Define docs target
if(MOCO_BUILD_DOCS)
  add_subdirectory(docs)
endif()
