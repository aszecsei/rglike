/**
 * @file world.cpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#include "world.hpp"

#include "constants.hpp"
#include <spdlog/spdlog.h>

namespace rglike {
    constexpr int DEFAULT_PLAYER_X_POS = 1;
    constexpr int DEFAULT_PLAYER_Y_POS = 1;

    void World::Initialize() {
        spdlog::info("Initializing world");

        m_player_pos.X() = DEFAULT_PLAYER_X_POS;
        m_player_pos.Y() = DEFAULT_PLAYER_Y_POS;
    }

    void World::Update() { }
} // namespace rglike
