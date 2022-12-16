/**
* @file game.hpp
* @author Alic Szecsei
* @date 12/15/2022
*/

#pragma once

#include "rglike/world.hpp"

namespace rglike {
    class Game {
    private:
        World m_world;
        int m_screen_width;
        int m_screen_height;

    public:
        void initialize();
        void run();
    };
} // namespace rglike
