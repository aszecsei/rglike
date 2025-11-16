cmake_minimum_required(VERSION 3.27)
project(rglike)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

# Fetch FTXUI
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v5.0.0
)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_MakeAvailable(ftxui)

# Fetch EnTT
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG v3.13.2
)
FetchContent_MakeAvailable(EnTT)

# Fetch Lua
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/marovira/lua.git
    GIT_TAG 5.4.4
)
FetchContent_MakeAvailable(lua)

# Fetch sol2
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2
    GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(sol2)

# Fetch fmt
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt
    GIT_TAG 11.0.2
)
FetchContent_MakeAvailable(fmt)

# Fetch spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.14.1
)
FetchContent_MakeAvailable(spdlog)

# Fetch CLI11
FetchContent_Declare(
        cli11_proj
        QUIET
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.6.1
)
FetchContent_MakeAvailable(cli11_proj)

# Add libraries subdirectory
add_subdirectory(libs)

# Add game subdirectory
add_subdirectory(game)
