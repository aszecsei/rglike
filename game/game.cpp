#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>
#include <CLI/Config.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <rglike/lib.hpp>

int main(int argc, char** argv) {
    std::string log_file_name = "log.txt";

    CLI::App app{"App description"};
    std::string filename = "default";
    app.add_option("-f,--file", filename, "A help string");

    CLI11_PARSE(app, argc, argv);

    // auto logger = spdlog::basic_logger_mt("basic_logger", log_file_name.c_str());

    rglike::Game game{};
    game.initialize();
    game.run();

    return 0;
}
