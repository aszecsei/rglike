#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <rglike/lib.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

auto main(int argc, char** argv) -> int {
    std::string log_filename = "log.txt";
    int width = 0;
    int height = 0;

    CLI::App app{"A small roguelike."};

    app.add_option("-l,--log", log_filename, "The file to use for logging");
    app.add_option("-x,--width", width, "Desired screen width (0 for fullscreen)");
    app.add_option("-y,--height", height, "Desired screen height (0 for fullscreen)");

    CLI11_PARSE(app, argc, argv);

    auto logger = spdlog::basic_logger_mt("basic_logger", log_filename);
    spdlog::set_default_logger(logger);

    rglike::Game game{};
    game.initialize();
    game.run();

    return EXIT_SUCCESS;
}
