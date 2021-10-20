cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED PROJECT_NAME)
  set(MOOUTILS_MASTER_PROJECT ON)
else()
  set(MOOUTILS_MASTER_PROJECT OFF)
endif()

project(MOOUTILS
  VERSION 0.1.0
  DESCRIPTION "Multi-Objective Optimization Utilities."
  LANGUAGES CXX
)

# Project options
option(MOOUTILS_BUILD_DOCS "Build the documentation" ON)
option(MOOUTILS_DEVELOPMENT_BUILD "Builds tests, benchmarks, enable warnings, etc.." OFF)

include(CMakeDependentOption)
cmake_dependent_option(MOOUTILS_BUILD_TESTS "Build tests" ON "MOOUTILS_DEVELOPMENT_BUILD" OFF)
cmake_dependent_option(MOOUTILS_BUILD_BENCHMARKS "Build benchmarks" ON "MOOUTILS_DEVELOPMENT_BUILD" OFF)
cmake_dependent_option(MOOUTILS_ENABLE_WERROR "Enable -Werror or /WX" OFF "MOOUTILS_DEVELOPMENT_BUILD" OFF)

# Set warning flags in development builds
if (MOOUTILS_DEVELOPMENT_BUILD)
  if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(MOOUTILS_CXX_WARN_FLAGS /W4)
    if(MOOUTILS_ENABLE_WERROR)
      list(APPEND MOOUTILS_CXX_WARN_FLAGS /WX)
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # TODO check if these all work in clang, or if we want different ones
    set(MOOUTILS_CXX_WARN_FLAGS
      -Wall
      -Wextra
      -Wpedantic
      -pedantic-errors
      -Wold-style-cast
      -Wundef
      -Wredundant-decls
      -Wcast-qual
      -Wmissing-include-dirs
      -Wdisabled-optimization
      -Wconversion
      -Wdouble-promotion
      -Wshadow
      -Wnull-dereference
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
      list(APPEND MOOUTILS_CXX_WARN_FLAGS -Wvector-operation-performance -Wduplicated-cond)
    endif()
    if(MOOUTILS_ENABLE_WERROR)
      list(APPEND MOOUTILS_CXX_WARN_FLAGS -Werror)
    endif()
  else()
    message(WARNING "Unknown compiler (not GNU, Clang, or MSVC), not setting any warning flags.")
    set(MOOUTILS_CXX_WARN_FLAGS "")
  endif()
else()
  set(MOOUTILS_CXX_WARN_FLAGS "")
endif()

# Extra stuff if this is the master project
if(MOOUTILS_MASTER_PROJECT)
  # Turn off C++ extensions
  set(CMAKE_CXX_EXTENSIONS OFF)
  # Create compile commands json for clang tools
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  # Include CTest
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

# Documentation
if(MOOUTILS_BUILD_DOCS)
  # TODO add_subdirectory(docs)
endif()

# Tests
if(MOOUTILS_BUILD_TESTS)
  add_subdirectory(tests)
endif()

# Benchmarks
if(MOOUTILS_BUILD_BENCHMARKS)
  # TODO add_subdirectory(benchmarks)
endif()

# PkgConfig file
configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/mooutils.pc.in
  ${PROJECT_BINARY_DIR}/mooutils.pc
  @ONLY
)
install(FILES "${PROJECT_BINARY_DIR}/mooutils.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# Export
install(TARGETS mooutils
        EXPORT mooutils-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

install(EXPORT mooutils-targets
        NAMESPACE mooutils::
        FILE mooutils-targets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mooutils
        )

include(CMakePackageConfigHelpers)
configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/mooutils-config.cmake.in
  "${PROJECT_BINARY_DIR}/mooutils-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mooutils
)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/mooutils-config-version.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMinorVersion
  )

install(FILES ${PROJECT_BINARY_DIR}/mooutils-config.cmake
              ${PROJECT_BINARY_DIR}/mooutils-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mooutils
        )
