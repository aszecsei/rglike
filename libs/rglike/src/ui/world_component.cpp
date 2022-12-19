/**
 * @file world_component.cpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#include "world_component.hpp"

#include "../game_log.hpp"
#include <fmt/core.h>

namespace rglike::ui {
    using namespace ftxui;

    /// @brief Responsible for rendering the game map.
    class WorldComponent : public ComponentBase {
    private:
        World& m_world;
        Box m_box;

    public:
        explicit WorldComponent(World& world)
            : m_world(world) { }

        [[nodiscard]] auto Render() -> ftxui::Element final;
        auto OnEvent(ftxui::Event event) -> bool final;

        [[nodiscard]] auto Focusable() const -> bool final { return true; }
    };

    auto WorldComponent::Render() -> ftxui::Element {
        ftxui::Elements world{};

        auto height = 1 + m_box.y_max - m_box.y_min;
        auto width = 1 + m_box.x_max - m_box.x_min;

        world.reserve(height);
        for (int i = 0; i < height; ++i) {
            ftxui::Elements row;
            row.reserve(width);
            for (int j = 0; j < width; ++j) {
                Vec2 pos{j, i};
                if (pos == m_world.PlayerPos()) {
                    row.push_back(ftxui::text("@"));
                } else {
                    row.push_back(ftxui::text("."));
                }
            }
            world.push_back(ftxui::hbox(row));
        }
        return ftxui::vbox(world) | flex | reflect(m_box);
    }

    auto WorldComponent::OnEvent(ftxui::Event event) -> bool {
        if (event.is_mouse() && m_box.Contain(event.mouse().x, event.mouse().y)) { TakeFocus(); }

        if (!Focused()) { return false; }

        if (event == ftxui::Event::Character(' ')) {
            auto height = 1 + m_box.y_max - m_box.y_min;
            auto width = 1 + m_box.x_max - m_box.x_min;
            GameLog::GetInstance().Log(fmt::format("World view size: ({}, {})", width, height));
            GameLog::GetInstance().Log(fmt::format("Player position: {}", m_world.PlayerPos()));
            return true;
        }

        if (event == ftxui::Event::ArrowUp) {
            m_world.MovePlayer(Vec2::Up());
            return true;
        }

        if (event == ftxui::Event::ArrowRight) {
            m_world.MovePlayer(Vec2::Right());
            return true;
        }

        if (event == ftxui::Event::ArrowDown) {
            m_world.MovePlayer(Vec2::Down());
            return true;
        }

        if (event == ftxui::Event::ArrowLeft) {
            m_world.MovePlayer(Vec2::Left());
            return true;
        }

        return false;
    }

    auto WorldViewer(World& gameLog) -> Component { return Make<WorldComponent>(gameLog); }
} // namespace rglike::ui
