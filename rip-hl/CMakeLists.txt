add_library(rip-hl STATIC)
target_compile_features(rip-hl PRIVATE cxx_std_20)
target_link_libraries(rip-hl PUBLIC simple-reflection stf)
add_subdirectory(src)
