add_library(rip-hl STATIC)
target_compile_features(rip-hl PRIVATE cxx_std_20)
target_link_libraries(rip-hl PUBLIC yyjson universal-cslib universal-cslib-reflection reflectcpp)
add_subdirectory(src)

add_executable(rip-hl-tests)
target_link_libraries(rip-hl-tests PRIVATE Catch2::Catch2WithMain rip-hl)
add_subdirectory(tests)

include(CTest)
include(Catch)
catch_discover_tests(rip-hl-tests)
