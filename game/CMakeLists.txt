add_executable(game game.cpp)
target_compile_features(game PRIVATE cxx_std_17)

target_link_libraries(game PRIVATE rglike fmt::fmt CLI11::CLI11)
