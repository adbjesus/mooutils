find_package(Catch2 REQUIRED)

add_executable(mooutils_tester
  main.cpp
  mooutils/indicators.cpp
  mooutils/sets.cpp
  mooutils/queues.cpp
  mooutils/solution.cpp
  mooutils/orders.cpp
)
target_link_libraries(mooutils_tester Catch2::Catch2)
target_link_libraries(mooutils_tester mooutils)
target_compile_definitions(mooutils_tester PUBLIC TEST_RESOURCES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_options(mooutils_tester PRIVATE ${MOOUTILS_CXX_WARN_FLAGS})

include(Catch)
catch_discover_tests(mooutils_tester)
