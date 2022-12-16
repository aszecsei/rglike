#pragma once

#include "rglike/world.hpp"

namespace rglike {
    class Game {
    private:
        World m_world;

    public:
        void initialize();
        void run();
    };
} // namespace rglike
