add_executable(rip)
target_compile_features(rip PRIVATE cxx_std_20)
target_link_libraries(rip PRIVATE rip-hl)
add_subdirectory(src)
