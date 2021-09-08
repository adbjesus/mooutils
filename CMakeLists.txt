cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED PROJECT_NAME)
  set(MOOUTILS_NOT_SUBPROJECT ON)
endif()

project(MOOUTILS
  VERSION 0.1.0
  DESCRIPTION "Library for Multi-Objective Combinatorial Optimization"
  LANGUAGES CXX
)

# Project options
# By default build apps and documentation
option(MOOUTILS_BUILD_APPS "Build the apps." ON)
option(MOOUTILS_BUILD_DOCS "Build the documentation." ON)
# By default build tests and benchmarks only if not a subproject
option(MOOUTILS_BUILD_TESTS "Build the tests." ${MOOUTILS_NOT_SUBPROJECT})
option(MOOUTILS_BUILD_BENCHES "Build the benchmarks." ${MOOUTILS_NOT_SUBPROJECT})

# Extra stuff if this is not being used as a subproject
if(MOOUTILS_NOT_SUBPROJECT)
  # Turn off C++ extensions
  set(CMAKE_CXX_EXTENSIONS OFF)
  # Create compile commands json for clang tools
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  # Include testing
  include(CTest)
endif()

# Install dirs for install targets
include(GNUInstallDirs)

# Define library target
add_library(mooutils INTERFACE)
target_compile_features(mooutils INTERFACE cxx_std_20)
target_include_directories(mooutils INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# The library is an install target
install(TARGETS mooutils)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/mooutils" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# PkgConfig file
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/mooutils.pc.in
  ${PROJECT_BINARY_DIR}/mooutils.pc
  @ONLY
)
install(FILES "${PROJECT_BINARY_DIR}/mooutils.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# Define solver target
if(MOOUTILS_BUILD_APPS)
  # add_subdirectory(apps)
endif()

# Define docs target
if(MOOUTILS_BUILD_DOCS)
  add_subdirectory(docs)
endif()

# Define tests target
if(MOOUTILS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Define tests target
if(MOOUTILS_BUILD_BENCHES)
  # TODO add_subdirectory(benches)
endif()
