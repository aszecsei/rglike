/**
 * @file game_log.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include <deque>
#include <ftxui/component/component.hpp>
#include <string>

using namespace ftxui;

namespace rglike {
    class GameLog : public ComponentBase {
    private:
        std::deque<std::string> m_log;
        int m_selected;
        Box m_box;

    public:
        void log(const std::string_view& text);
        [[nodiscard]] auto Render() -> ftxui::Element final;
        auto OnEvent(Event event) -> bool final;

        [[nodiscard]] auto Focusable() const -> bool final { return true; }

        [[nodiscard]] inline auto selected() const -> int { return m_selected; }
    };
} // namespace rglike