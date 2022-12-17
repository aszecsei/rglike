/**
* @file game.cpp
* @author Alic Szecsei
* @date 12/15/2022
*/


#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <rglike/rglike.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

auto main(int argc, char** argv) -> int {
    std::string log_filename = "log.txt";

    CLI::App app{"A small roguelike."};

    app.add_option("-l,--log", log_filename, "The file to use for logging");

    CLI11_PARSE(app, argc, argv);

    auto logger = spdlog::basic_logger_mt("basic_logger", log_filename);
    spdlog::set_default_logger(logger);

    rglike::Game game{};
    game.Initialize();
    game.Run();

    return EXIT_SUCCESS;
}
