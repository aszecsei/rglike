#include "rglike/lib.hpp"

#include <spdlog/spdlog.h>

namespace rglike {
    void Game::initialize() {
        spdlog::info("Initializing...");
    }

    void Game::run() {
        spdlog::info("Running...");
    }
}