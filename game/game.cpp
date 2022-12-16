#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <rglike/lib.hpp>

auto main(int argc, char** argv) -> int {
    std::string log_filename = "log.txt";

    CLI::App app{"App description"};
    app.add_option("-l,--log", log_filename, "The file to use for logging");

    CLI11_PARSE(app, argc, argv);

    auto logger = spdlog::basic_logger_mt("basic_logger", log_filename);
    spdlog::set_default_logger(logger);

    rglike::Game game{};
    game.initialize();
    game.run();

    return 0;
}
