/**
 * @file game_log.hpp
 * @author Alic Szecsei
 * @date 12/16/2022
 */

#pragma once

#include <deque>
#include <ftxui/component/component.hpp>
#include <string>
#include <utility>
#include <vector>

using namespace ftxui;

namespace rglike {
    struct GameLogFragment {
        Color color;
        Color bg_color;
        bool bold;
        bool dim;
        bool underlined;
        bool blinking;
        std::string text;

        GameLogFragment(
            Color color, Color bg_color, bool bold, bool dim, bool underlined, bool blinking,
            const std::string_view& text
        )
            : color(color)
            , bg_color(bg_color)
            , bold(bold)
            , dim(dim)
            , underlined(underlined)
            , blinking(blinking)
            , text(text) { }
    };

    auto logParagraph(const std::vector<GameLogFragment>& the_text) -> ftxui::Element;
    auto logParagraphAlignLeft(const std::vector<GameLogFragment>& the_text) -> ftxui::Element;
    /*
    auto logParagraphAlignRight(const std::vector<GameLogFragment>& the_text) -> ftxui::Element;
    auto logParagraphAlignCenter(const std::vector<GameLogFragment>& the_text) -> ftxui::Element;
    auto logParagraphAlignJustify(const std::vector<GameLogFragment>& the_text) -> ftxui::Element;
    */

    class GameLog;

    class LogEntry {
    private:
        Color m_current_color = Color::Default;
        Color m_current_bg_color = Color::Default;
        bool m_current_bold = false;
        bool m_current_dim = false;
        bool m_current_underlined = false;
        bool m_current_blinking = false;

        std::vector<GameLogFragment> m_fragments;
        GameLog* m_log;

        explicit LogEntry(GameLog* log)
            : m_log(log) { }

        [[nodiscard]] inline auto fragments() const -> std::vector<GameLogFragment> {
            return m_fragments;
        }

    public:
        [[nodiscard]] auto color(Color color) -> LogEntry&;
        [[nodiscard]] auto bg_color(Color color) -> LogEntry&;
        [[nodiscard]] auto bold(bool bold) -> LogEntry&;
        [[nodiscard]] auto dim(bool dim) -> LogEntry&;
        [[nodiscard]] auto underline(bool underline) -> LogEntry&;
        [[nodiscard]] auto blink(bool blink) -> LogEntry&;
        [[nodiscard]] auto text(const std::string_view& text) -> LogEntry&;
        void log() const;

        friend class GameLog;
    };

    class GameLog : public ComponentBase {
    private:
        std::deque<std::vector<GameLogFragment>> m_log;
        int m_selected;
        Box m_box;

        void log(const LogEntry& entry);

    public:
        auto entry() -> LogEntry;
        void log(const std::string_view& text);

        [[nodiscard]] auto Render() -> ftxui::Element final;
        auto OnEvent(Event event) -> bool final;

        [[nodiscard]] auto Focusable() const -> bool final { return true; }

        [[nodiscard]] inline auto selected() const -> int { return m_selected; }

        friend class LogEntry;
    };
} // namespace rglike