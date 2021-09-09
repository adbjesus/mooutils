find_package(fmt REQUIRED)
find_package(Boost REQUIRED)

add_executable(mobkp_pls mobkp_pls.cpp)
target_link_libraries(mobkp_pls mooutils fmt::fmt Boost::headers)
target_compile_options(mobkp_pls PRIVATE ${MOOUTILS_CXX_WARN_FLAGS})
