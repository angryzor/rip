cmake_minimum_required(VERSION 3.28)
project(rip VERSION 1.0.2
                 DESCRIPTION "Restoration Issue Pocketknife"
                 LANGUAGES CXX)

add_subdirectory(vendor EXCLUDE_FROM_ALL)
add_subdirectory(rip)
add_subdirectory(rip-hl)
