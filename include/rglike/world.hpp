/**
 * @file world.hpp
 * @author Alic Szecsei
 * @date 12/15/2022
 */

#pragma once

#include <entt/entt.hpp>
#include <ftxui/dom/elements.hpp>

namespace rglike {
    class World {
    private:
        int m_player_x;
        int m_player_y;

    public:
        entt::registry Registry;

        inline void set_player_x(int x) { m_player_x = x; }

        inline void set_player_y(int y) { m_player_y = y; }

        inline void move_player_x(int delta) { m_player_x += delta; }

        inline void move_player_y(int delta) { m_player_y += delta; }

        void initialize();

        void update();
        [[nodiscard]] auto render(int width, int height) const -> ftxui::Element;
    };
} // namespace rglike
