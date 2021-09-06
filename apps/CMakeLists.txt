find_package(fmt REQUIRED)

add_executable(mobkp_pls mobkp_pls.cpp)
target_link_libraries(mobkp_pls moco fmt::fmt)
target_compile_options(mobkp_pls PRIVATE ${MOCO_CXX_WARN_FLAGS})
