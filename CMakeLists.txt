cmake_minimum_required(VERSION 3.28)
project(rip VERSION 0.0.3
                 DESCRIPTION "Sonic Team Formats"
                 LANGUAGES CXX)

add_subdirectory(vendor EXCLUDE_FROM_ALL)
add_subdirectory(rip)
add_subdirectory(rip-hl)
