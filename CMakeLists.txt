cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED PROJECT_NAME)
  set(MOUTILS_NOT_SUBPROJECT ON)
endif()

project(MOUTILS
  VERSION 0.1.0
  DESCRIPTION "Library for Multi-Objective Combinatorial Optimization"
  LANGUAGES CXX
)

# Project options
# By default build apps and documentation
option(MOUTILS_BUILD_APPS "Build the apps." ON)
option(MOUTILS_BUILD_DOCS "Build the documentation." ON)
# By default build tests and benchmarks only if not a subproject
option(MOUTILS_BUILD_TESTS "Build the tests." ${MOUTILS_NOT_SUBPROJECT})
option(MOUTILS_BUILD_BENCHES "Build the benchmarks." ${MOUTILS_NOT_SUBPROJECT})

# Extra stuff if this is not being used as a subproject
if(MOUTILS_NOT_SUBPROJECT)
  # Turn off C++ extensions
  set(CMAKE_CXX_EXTENSIONS OFF)
  # Create compile commands json for clang tools
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  # Include testing
  include(CTest)
endif()

# Define library target
add_library(moutils INTERFACE)
target_compile_features(moutils INTERFACE cxx_std_20)
target_include_directories(moutils INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# The library is an install target
install(TARGETS moutils)

# Define solver target
if(MOUTILS_BUILD_APPS)
  add_subdirectory(apps)
endif()

# Define docs target
if(MOUTILS_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# Define tests target
if(MOUTILS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Define tests target
if(MOUTILS_BUILD_BENCHES)
  # TODO add_subdirectory(benches)
endif()
