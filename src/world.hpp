/**
 * @file world.hpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#pragma once

#include "entt/entt.hpp"
#include "ftxui/dom/elements.hpp"

namespace rglike {
    class World {
    private:
        int m_player_x;
        int m_player_y;

    public:
        entt::registry Registry;

        inline void SetPlayerX(int x) { m_player_x = x; }

        inline void SetPlayerY(int y) { m_player_y = y; }

        inline void MovePlayerX(int delta) { m_player_x += delta; }

        inline void MovePlayerY(int delta) { m_player_y += delta; }

        void Initialize();

        void Update();
        [[nodiscard]] auto Render(int width, int height) const -> ftxui::Element;
    };
} // namespace rglike
