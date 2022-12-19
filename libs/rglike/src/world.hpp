/**
 * @file world.hpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#pragma once

#include "math.hpp"

#include "entt/entt.hpp"
#include "ftxui/dom/elements.hpp"

namespace rglike {
    class World {
    private:
        Vec2 m_player_pos{0, 0};

    public:
        entt::registry Registry;

        [[nodiscard]] inline auto PlayerPos() const -> Vec2 { return m_player_pos; }

        inline void MovePlayer(Vec2 dir) { m_player_pos += dir; }

        void Initialize();

        void Update();
    };
} // namespace rglike
