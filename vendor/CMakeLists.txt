include(FetchContent)
FetchContent_Declare(
    yyjson
    GIT_REPOSITORY https://github.com/ibireme/yyjson.git
    GIT_TAG c3af82ba8d72125469d956fe8c9f0de5b3b8f5e7
    FIND_PACKAGE_ARGS
)
FetchContent_Declare(
    reflectcpp
    GIT_REPOSITORY https://github.com/getml/reflect-cpp.git
    GIT_TAG v0.16.0
    FIND_PACKAGE_ARGS
)
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.4.2
    FIND_PACKAGE_ARGS 2.4
)
FetchContent_Declare(
    universal-cslib
    GIT_REPOSITORY https://github.com/angryzor/universal-cslib.git
    GIT_TAG v1.0.2
    FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(yyjson reflectcpp CLI11 universal-cslib)
