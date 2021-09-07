find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

add_executable(moutils_tester
  main.cpp
  moutils/util/dominance.cpp
  moutils/util/solution_sets.cpp
  moutils/util/solution_queues.cpp
  moutils/problems/mobkp.cpp
)
target_link_libraries(moutils_tester Catch2::Catch2)
target_link_libraries(moutils_tester fmt::fmt)
target_link_libraries(moutils_tester moutils)
#target_compile_definitions(moutils_tester PUBLIC TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_options(moutils_tester PRIVATE ${MOUTILS_CXX_WARN_FLAGS})

include(Catch)
catch_discover_tests(moutils_tester)
