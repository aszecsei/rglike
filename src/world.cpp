/**
 * @file world.cpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#include "rglike/world.hpp"

#include "rglike/constants.hpp"
#include <spdlog/spdlog.h>

namespace rglike {
    constexpr int DEFAULT_PLAYER_X_POS = 1;
    constexpr int DEFAULT_PLAYER_Y_POS = 1;

    void World::initialize() {
        spdlog::info("Initializing world");

        m_player_x = DEFAULT_PLAYER_X_POS;
        m_player_y = DEFAULT_PLAYER_Y_POS;
    }

    void World::update() { }

    auto World::render(int width, int height) const -> ftxui::Element {
        ftxui::Elements world{};
        world.reserve(height);
        for (int i = 0; i < height; ++i) {
            ftxui::Elements row;
            row.reserve(width);
            for (int j = 0; j < width; ++j) {
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
