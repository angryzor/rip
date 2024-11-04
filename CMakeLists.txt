cmake_minimum_required(VERSION 3.28)
project(rip VERSION 0.0.1
                 DESCRIPTION "Sonic Team Formats"
                 LANGUAGES CXX)

add_subdirectory(../sonic-team-formats stf EXCLUDE_FROM_ALL)
add_subdirectory(rip)
add_subdirectory(rip-hl)
