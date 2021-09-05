find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

add_executable(moco_tester
  main.cpp
  moco/util/dominance.cpp
  moco/util/solution_sets.cpp
  moco/util/solution_queues.cpp
  moco/problems/mobkp.cpp
)
target_link_libraries(moco_tester Catch2::Catch2)
target_link_libraries(moco_tester fmt::fmt)
target_link_libraries(moco_tester moco)
#target_compile_definitions(moco_tester PUBLIC TEST_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
target_compile_options(moco_tester PRIVATE ${MOCO_CXX_WARN_FLAGS})

include(Catch)
catch_discover_tests(moco_tester)
