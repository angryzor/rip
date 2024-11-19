add_library(rip-hl STATIC)
target_compile_features(rip-hl PRIVATE cxx_std_20)
target_link_libraries(rip-hl PUBLIC yyjson simple-reflection universal-cslib universal-cslib-reflection reflectcpp)
add_subdirectory(src)
