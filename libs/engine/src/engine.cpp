#include "engine/engine.h"
#include "engine/lua_bindings.h"
#include <ftxui/component/screen_interactive.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>

namespace engine {

Engine::Engine() {
    // Set up logger with both console and file output
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/engine.log", true);

    logger_ = std::make_shared<spdlog::logger>("engine", spdlog::sinks_init_list{console_sink, file_sink});
    logger_->set_level(spdlog::level::debug);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

    spdlog::set_default_logger(logger_);

    logger_->info("Engine initialized");

    // Initialize Lua bindings
    LuaBindings::initialize(lua_, this, logger_);
    logger_->info("Lua initialized");
}

Engine::~Engine() {
    shutdown();
}

void Engine::load_data_files(const std::string& data_dir) {
    namespace fs = std::filesystem;

    if (!fs::exists(data_dir) || !fs::is_directory(data_dir)) {
        logger_->error("Data directory '{}' does not exist or is not a directory", data_dir);
        return;
    }

    logger_->info("Loading data files from '{}'", data_dir);

    // Recursively iterate through all .lua files in the data directory
    for (const auto& entry : fs::recursive_directory_iterator(data_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            const auto& path = entry.path();
            logger_->info("Loading Lua script: {}", path.string());

            try {
                lua_.script_file(path.string());
                logger_->debug("Successfully loaded: {}", path.string());
            } catch (const sol::error& e) {
                logger_->error("Error loading {}: {}", path.string(), e.what());
            }
        }
    }

    logger_->info("Finished loading data files");
}

void Engine::run() {
    running_ = true;
    logger_->info("Starting engine main loop");

    if (scene_manager_.empty()) {
        logger_->warn("No scenes in scene stack, engine will display empty screen");
    }

    auto screen = ftxui::ScreenInteractive::Fullscreen();
    screen_ = &screen;

    // Create a component that updates scene logic before rendering
    auto component = ftxui::Renderer(scene_manager_.get_component(), [this] {
        scene_manager_.update();  // Update scene logic each frame
        return scene_manager_.get_component()->Render();
    });

    screen.Loop(component);

    screen_ = nullptr;
    logger_->info("Engine main loop ended");
}

void Engine::shutdown() {
    if (running_) {
        logger_->info("Shutting down engine");
        running_ = false;
        if (screen_) {
            screen_->Exit();
        }
    }
}

} // namespace engine
