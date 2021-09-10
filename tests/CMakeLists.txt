find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

add_executable(mooutils_tester
  main.cpp
  mooutils/dominance.cpp
  mooutils/indicators.cpp
  mooutils/sets.cpp
  mooutils/queues.cpp
  mooutils/problems/mobkp.cpp
)
target_link_libraries(mooutils_tester Catch2::Catch2)
target_link_libraries(mooutils_tester fmt::fmt)
target_link_libraries(mooutils_tester mooutils)
#target_compile_definitions(mooutils_tester PUBLIC TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_options(mooutils_tester PRIVATE ${MOOUTILS_CXX_WARN_FLAGS})

include(Catch)
catch_discover_tests(mooutils_tester)
