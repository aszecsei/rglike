#include "rglike/world.hpp"

#include "rglike/constants.hpp"
#include <spdlog/spdlog.h>

namespace rglike {
    void World::initialize() {
        spdlog::info("Initializing world");
        m_player_x = 10;
        m_player_y = 10;
    }

    void World::update() { }

    auto World::render() const -> ftxui::Element {
        ftxui::Elements world;
        world.reserve(SCREEN_HEIGHT);
        for (int i = 0; i < SCREEN_HEIGHT; ++i) {
            ftxui::Elements row;
            row.reserve(SCREEN_WIDTH);
            for (int j = 0; j < SCREEN_WIDTH; ++j) {
                if (i == m_player_y && j == m_player_x) {
                    row.push_back(ftxui::text("@"));
                } else {
                    row.push_back(ftxui::text("."));
                }
            }
            world.push_back(ftxui::hbox(row));
        }
        return ftxui::vbox(world);
    }
} // namespace rglike
